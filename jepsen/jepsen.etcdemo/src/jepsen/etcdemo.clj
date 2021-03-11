(ns jepsen.etcdemo
  (:require [jepsen.cli :as cli]
            [jepsen.tests :as tests]))

(defn etcd-test
  ""
  [opts]
  (merge tests/noop-test
         {:pure-generators true}
         opts))

(defn -main
  "Handles command line arguments. Can either run a test, or a web server for
  browsing results."
  [& args]
  (cli/run! (merge (cli/single-test-cmd {:test-fn etcd-test})
                   (cli/serve-cmd))
            args))