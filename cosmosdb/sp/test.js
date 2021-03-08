function sample(rec) {
    var collection = getContext().getCollection();
    //var rec = {'id': "xxx", 'balance': 99};

    function merge(source, update) {
        for (var column in update) {
            if (update[column].constructor == Object) {
                source[column] = merge(source[column], update[column]);
            } else {
                source[column] = update[column];
                console.log("HELLO");
            }
        }
        return source;
    }

    // Query documents and take 1st item.
    var isAccepted = __.queryDocuments(
        __.getSelfLink(),
        'SELECT * FROM account a where a.id = "xxx"',
        function (err, feed, options) {
            if (err) throw err;

            // Check the feed and if empty, set the body to 'no docs found', 
            // else take 1st element from feed
            if (!feed || !feed.length) {
                var response = getContext().getResponse();
                response.setBody('no docs found');
            }
            else {
                var response = getContext().getResponse();
                var body = { record: rec, feed: feed };
                response.setBody(JSON.stringify(body));
                const upserted = merge(feed[0], rec);
                //response.setBody(JSON.stringify(feed[0]));

                const isMutationAccepted = __.upsertDocument(__.getSelfLink(), upserted,
                    (error, result, options) => {
                        if (error) throw error;
                    });
                if (!isMutationAccepted) {
                    reject(new Error("The query was not accepted by the server."));
                }
            }
        }
    );

    if (!isAccepted) throw new Error('The query was not accepted by the server.');

}
