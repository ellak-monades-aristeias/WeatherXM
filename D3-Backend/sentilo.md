install java
install mvn

Installing Maven
Add a 3rd party PPA with Maven:
sudo add-apt-repository "deb http://ppa.launchpad.net/natecarlson/maven3/ubuntu precise main"
Update software packages with a new repository:
sudo apt-get update
Install Maven3:
sudo apt-get install maven3
Add a symlink so that the maven command is in the system path:
sudo ln -s /usr/share/maven3/bin/mvn /usr/bin/mvn




chmod +x



=======================================
Step 1: Compile and build platform code
=======================================
This command will compile and build platform code via Maven:

mvn clean install

Press enter when ready.

…..downloads dependencies….builds...tests…






Tests run: 38, Failures: 1, Errors: 0, Skipped: 0

[INFO] ------------------------------------------------------------------------
[INFO] Reactor Summary:
[INFO]
[INFO] sentilo-parent-pom ................................ SUCCESS [  2.984 s]
[INFO] sentilo-common .................................... SUCCESS [  8.227 s]
[INFO] sentilo-agent-common .............................. SUCCESS [  2.185 s]
[INFO] sentilo-platform .................................. SUCCESS [  0.007 s]
[INFO] sentilo-platform-common ........................... SUCCESS [  0.667 s]
[INFO] sentilo-platform-service .......................... SUCCESS [  3.189 s]
[INFO] sentilo-platform-server ........................... SUCCESS [  7.173 s]
[INFO] sentilo-platform-client-java ...................... SUCCESS [  2.261 s]
[INFO] sentilo-agent-alarm ............................... FAILURE [  1.808 s]
[INFO] sentilo-agent-relational .......................... SKIPPED
[INFO] sentilo-catalog-web ............................... SKIPPED
[INFO] sentilo-agent-location-updater .................... SKIPPED
[INFO] ------------------------------------------------------------------------
[INFO] BUILD FAILURE
[INFO] ------------------------------------------------------------------------
[INFO] Total time: 28.756 s
[INFO] Finished at: 2015-10-08T18:09:00-06:00
[INFO] Final Memory: 32M/205M
[INFO] ------------------------------------------------------------------------
[ERROR] Failed to execute goal org.apache.maven.plugins:maven-surefire-plugin:2.12.4:test (default-test) on project sentilo-agent-alert: There are test failures.
[ERROR]
[ERROR] Please refer to /opt/sentilo/sentilo-agent-alert/target/surefire-reports for the individual test results.
[ERROR] -> [Help 1]
[ERROR]
[ERROR] To see the full stack trace of the errors, re-run Maven with the -e switch.
[ERROR] Re-run Maven using the -X switch to enable full debug logging.
[ERROR]
[ERROR] For more information about the errors and possible solutions, please read the following articles:
[ERROR] [Help 1] http://cwiki.apache.org/confluence/display/MAVEN/MojoFailureException
[ERROR]
[ERROR] After correcting the problems, you can resume the build with the command
[ERROR]   mvn <goals> -rf :sentilo-agent-alert
Error code : 1






We are not going to use the agent-alarm (for now) so lets try skipping the tests
mvn install -Dmaven.test.skip=true

 cd sentilo-agent-location-updater/
cd sentilo-catalog-web/
cd sentilo-agent-relational/



need to install tomcat
sudo apt-get install tomcat7
sudo apt-get install tomcat7-admin


sudo nano /etc/tomcat7/tomcat-users.xml

<tomcat-users>
    <user username="admin" password="password" roles="manager-gui,admin-gui"/>
</tomcat-users>


sudo service tomcat7 restart

http://server_IP_address:8080
sudo add-apt-repository ppa:chris-lea/redis-server

 redis-cli ping
PONG

/etc/redis/redis.conf
chance requirepass foobar to sentilo

sudo service redis-server restart




http://docs.mongodb.org/manual/tutorial/install-mongodb-on-ubuntu/

enable authendication

mongo


> use admin
switched to db admin
> db.addUser( { user:"sentilo", pwd:"sentilo", roles: ["userAdminAnyDatabase"]})
WARNING: The 'addUser' shell helper is DEPRECATED. Please use 'createUser' instead
Successfully added user: { "user" : "sentilo", "roles" : [ "userAdminAnyDatabase" ] }


db.createUser ( { user:"sentilo", pwd:"sentilo", roles: ["userAdminAnyDatabase"]})

/etc/mongod.conf
auth=true

service mongod restart

root@oldserver:/opt/sentilo/scripts/mongodb# mongo -u sentilo -p sentilo sentilo init_data.js  --authenticationDatabase admin
MongoDB shell version: 2.6.11
connecting to: sentilo
Load applications
Load users
Load permissions

cp sentilo-catalog-web.war /usr/share/tomcat7-root/


sentilo-platform/sentilo-platform-server
mvn package appassembler:assemble -P dev

chmod +x



https://github.com/sentilo/sentilo/issues/33
apparently, it will not work with mongo 3!

