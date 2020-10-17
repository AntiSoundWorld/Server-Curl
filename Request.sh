curl 127.0.0.1:8090

curl 127.0.0.1:8090 -d '{"name":"Dmitry"}' 

curl -X DELETE 127.0.0.1:8090?id=0

curl  -X PUT '127.0.0.1:8090?id=0&name=Dmitry'
