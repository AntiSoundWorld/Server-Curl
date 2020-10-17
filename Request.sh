curl 127.0.0.1:8090

curl -H 'Content-Type: application/json' 127.0.0.1:8090 -d '{"name":"Dmitry"}'

curl -X DELETE -H 'Content-Type: application/json' '127.0.0.1:8090?id=0'

сurl -X PUT -H 'Content-Type: application/json' '127.0.0.1:8090?id=0&name=Slava'
