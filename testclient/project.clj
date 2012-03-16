(defproject testclient "test client for playpausestop"
  :description "work in progress"
  :dependencies [[org.clojure/clojure "1.3.0"]
                 [com.leadtune/clojure-zmq "2.1.0"]]
  :native-dependencies [[org.clojars.starry/jzmq-native-deps "2.0.10.4"]]
  :dev-dependencies [[native-deps "1.0.1"]]
  :main testclient.core)
