curl 127.0.0.1:8090

curl 127.0.0.1:8090 -H Content-Type: application/json -d '{"name":"Dmitry"}' 

curl -v 127.0.0.1:8090 -X DELETE -H Content-Type: application/json -d '{"id":"0"}'

curl 127.0.0.1:8090 -X PUT -H Content-Type: application/json -d '{"Id":"0","name":"Anastasia"}'
