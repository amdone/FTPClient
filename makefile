ftpclient:ftp_client.c ftp.h
	gcc  ftp_client.c -o ftp_cli
clean:
	rm -f ftp_cli