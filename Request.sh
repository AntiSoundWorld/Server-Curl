curl 127.0.0.1:8090

curl 127.0.0.1:8090 -d '{"name":"Dmitry"}' 

curl -v 127.0.0.1:8090 -X DELETE -H Content-Type: application/json -d '{"id":"0"}'

curl 127.0.0.1:8090 -X PUT -d '{"Id":"0","name":"Anastasia"}'
