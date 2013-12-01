ideaExcludeFolders += ".idea"

ideaExcludeFolders += ".idea_modules"

libraryDependencies +=
  "org.json4s" %% "json4s-native" % "3.2.5"

libraryDependencies += "org.apache.commons" % "commons-io" % "1.3.2"

resolvers += "spray" at "http://repo.spray.io/"

libraryDependencies += "io.spray" %%  "spray-json" % "1.2.5"

libraryDependencies += "org.apache.httpcomponents" % "fluent-hc" % "4.3.1"

libraryDependencies += "org.apache.httpcomponents" % "httpmime" % "4.3.1"
