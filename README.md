# webserv
## echo server
**--使い方--**
```bash
make -C server && ./server/server &; c++ client.c -o client && ./client
```
現状, サーバープログラムは動き続ける。
EOFを送信するか、^Dを打つかでclientとの接続が切れ、次の待機キューとの接続を試みる。
