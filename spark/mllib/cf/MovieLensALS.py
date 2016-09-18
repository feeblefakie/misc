#!/usr/bin/env python

import sys
import itertools
import time
from math import sqrt
from operator import add
from os.path import join, isfile, dirname

from pyspark import SparkConf, SparkContext
from pyspark.sql import SQLContext
from pyspark.mllib.recommendation import ALS

def parseRating(line):
    # Parses a rating record in MovieLens format userId::movieId::rating::timestamp .
    fields = line.strip().split("::")
    return long(fields[3]) % 10, (int(fields[0]), int(fields[1]), float(fields[2]))

def parseMovie(line):
    # Parses a movie record in MovieLens format movieId::movieTitle .
    fields = line.strip().split("::")
    return int(fields[0]), fields[1]

def loadMovieFromC(sc):
    sqlContext = SQLContext(sc)
    df = sqlContext.read.format("org.apache.spark.sql.cassandra").options(table="movies", keyspace="test").load()
    df.registerTempTable("movies")
    return sqlContext.sql("SELECT * FROM movies").rdd;

def loadRatingsFromC(sc):
    sqlContext = SQLContext(sc)
    df = sqlContext.read.format("org.apache.spark.sql.cassandra").options(table="ratings", keyspace="test").load()
    df.registerTempTable("ratings")
    return sqlContext.sql("SELECT * FROM ratings").rdd;

def loadRatings(ratingsFile):
    # Load ratings from file.
    if not isfile(ratingsFile):
        print "File %s does not exist." % ratingsFile
        sys.exit(1)
    f = open(ratingsFile, 'r')
    ratings = filter(lambda r: r[2] > 0, [parseRating(line)[1] for line in f])
    f.close()
    if not ratings:
        print "No ratings provided."
        sys.exit(1)
    else:
        return ratings

def computeRmse(model, data, n):
    # Compute RMSE (Root Mean Squared Error).
    predictions = model.predictAll(data.map(lambda x: (x[0], x[1])))
    predictionsAndRatings = predictions.map(lambda x: ((x[0], x[1]), x[2])) \
      .join(data.map(lambda x: ((x[0], x[1]), x[2]))) \
      .values()
    return sqrt(predictionsAndRatings.map(lambda x: (x[0] - x[1]) ** 2).reduce(add) / float(n))

if __name__ == "__main__":
    if (len(sys.argv) != 3):
        print "Usage: /path/to/spark/bin/spark-submit --driver-memory 2g " + \
          "MovieLensALS.py movieLensDataDir personalRatingsFile"
        sys.exit(1)

    # set up environment
    conf = SparkConf() \
      .setAppName("MovieLensALS") \
      .set("spark.executor.memory", "2g")
    sc = SparkContext(conf=conf)

    # load personal ratings
    #myRatings = loadRatings(sys.argv[2])
    #myRatingsRDD = sc.parallelize(myRatings, 1)
    
    # load ratings and movie titles

    movieLensHomeDir = sys.argv[1]

    # ratings is an RDD of (last digit of timestamp, (userId, movieId, rating))
    #ratings = sc.textFile(join(movieLensHomeDir, "ratings.dat")).map(parseRating)
    ratings = loadRatingsFromC(sc).map(lambda r: (long(r[3]) % 10, (r[0], r[1], r[2])))

    # movies is an RDD of (movieId, movieTitle)
    #movies = dict(sc.textFile(join(movieLensHomeDir, "movies.dat")).map(parseMovie).collect())

    numRatings = ratings.count()
    users = ratings.values().map(lambda r: r[0]).distinct()
    #for x in users_movies.collect():
    #    print "result %s" % (x[0],)
    #sys.exit(1)

    numUsers = users.count()
    #numMovies = ratings.values().map(lambda r: r[1]).distinct().count()

    #for x in users.collect():
    #    print x
    #sys.exit(1)

    #print "Got %d ratings from %d users on %d movies." % (numRatings, numUsers, numMovies)
    print "Got %d ratings from %d users." % (numRatings, numUsers)

    # split ratings into train (60%), validation (20%), and test (20%) based on the 
    # last digit of the timestamp, add myRatings to train, and cache them

    # training, validation, test are all RDDs of (userId, movieId, rating)

    numPartitions = 16
    training = ratings.filter(lambda x: x[0] < 6) \
      .values() \
      .repartition(numPartitions) \
      .cache()

    validation = ratings.filter(lambda x: x[0] >= 6 and x[0] < 8) \
      .values() \
      .repartition(numPartitions) \
      .cache()

    test = ratings.filter(lambda x: x[0] >= 8).values().cache()

    numTraining = training.count()
    numValidation = validation.count()
    numTest = test.count()

    print "Training: %d, validation: %d, test: %d" % (numTraining, numValidation, numTest)

    #
    # train models and evaluate them on the validation set
    #

    # dimension
    #ranks = [8, 12]
    ranks = [8]
    # regulation parameter
    #lambdas = [0.1, 10.0]
    lambdas = [0.1]
    #numIters = [10, 20]
    numIters = [10]
    bestModel = None
    bestValidationRmse = float("inf")
    bestRank = 0
    bestLambda = -1.0
    bestNumIter = -1

    model_start = time.time()

    for rank, lmbda, numIter in itertools.product(ranks, lambdas, numIters):
        #model = ALS.train(training, rank, numIter, lmbda)
        model = ALS.trainImplicit(training, rank, numIter, lmbda)
        validationRmse = computeRmse(model, validation, numValidation)
        print "RMSE (validation) = %f for the model trained with " % validationRmse + \
              "rank = %d, lambda = %.1f, and numIter = %d." % (rank, lmbda, numIter)
        if (validationRmse < bestValidationRmse):
            bestModel = model
            bestValidationRmse = validationRmse
            bestRank = rank
            bestLambda = lmbda
            bestNumIter = numIter

    testRmse = computeRmse(bestModel, test, numTest)

    model_end = time.time()

    # evaluate the best model on the test set
    print "The best model was trained with rank = %d and lambda = %.1f, " % (bestRank, bestLambda) \
      + "and numIter = %d, and its RMSE on the test set is %f." % (bestNumIter, testRmse)

    # compare the best model with a naive baseline that always returns the mean rating
    meanRating = training.union(validation).map(lambda x: x[2]).mean()
    baselineRmse = sqrt(test.map(lambda x: (meanRating - x[2]) ** 2).reduce(add) / numTest)
    #improvement = (baselineRmse - testRmse) / baselineRmse * 100
    #print "The best model improves the baseline by %.2f" % (improvement) + "%."

    #
    # make personalized recommendations
    #

    #myRatedMovieIds = set([x[1] for x in myRatings])
    # extracting keys from movies and create a RDD
    #candidates = sc.parallelize([m for m in movies if m not in myRatedMovieIds])
    # 0 means user 0, a RDD of (userId, movieId)
    # result is a RDD of (userId, movieId, rating)
    # candidates will be from getting cross product of users and products except for items which user actually had actions on products
    #predictions = bestModel.predictAll(candidates.map(lambda x: (0, x))).collect()

    # sort the result by rating (what if there are multiple users ?)
    #recommendations = sorted(predictions, key=lambda x: x[2], reverse=True)[:50]

    #print "Movies recommended for you:"
    #for i in xrange(len(recommendations)):
    #    print ("%2d: %s %lf" % (i + 1, movies[recommendations[i][1]], recommendations[i][2])).encode('ascii', 'ignore')

    #movies = sc.textFile(join(movieLensHomeDir, "movies.dat")).map(parseMovie)
    movies = loadMovieFromC(sc)
    users_movies = users.cartesian(movies.map(lambda x: x[0]))

    predictions = bestModel.predictAll(users_movies)

    count = predictions.map(lambda (userId, movieId, rating): (userId, movieId)).count()
    print "count %d" % count
    rec_end = time.time()
    #predictions.map(lambda x: (x[0], x[1], x[2])).saveAsTextFile("/tmp/spark")
    #predictions.groupBy(lambda x: x[0]).saveAsTextFile("/tmp/spark")
    predictions.sortBy(lambda x: (x[0], x[2])).saveAsTextFile("/tmp/spark")
    print_end = time.time()

    #for i in range(1, 6041):
    #    try:
    #        recommendations = bestModel.recommendProducts(i, 5)
    #        for i in xrange(len(recommendations)):
    #            print ("%2d: %s %lf" % (i + 1, movies[recommendations[i][1]], recommendations[i][2])).encode('ascii', 'ignore')
    #    except:
    #        print ("no recommendations for %d\n" % (i)).encode('ascii', 'ignore')

    print("model: %lf" % (model_end - model_start))
    print("recommend: %lf" % (rec_end - model_end))
    print("print: %lf" % (print_end - rec_end))
        
    # clean up
    sc.stop()

