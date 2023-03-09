if [ ! -f "ca.key" ];then
openssl genrsa -out ca.key 2048
openssl req -new -key ca.key -out ca.csr -subj "/C=US/ST=CA/L=LA/O=Talus/OU=Talus/CN=Talus_CA"
openssl x509 -req -days 10000 -sha1 -extensions v3_ca -signkey ca.key -in ca.csr -out ca.cer
fi

openssl genrsa -out certificate.key 2048
openssl req -new -key certificate.key -out certificate.csr -subj "/C=US/ST=CA/L=LA/O=Talus/OU=Talus/CN=*"
openssl x509 -req -days 3650 -sha1 -extensions v3_req  -CA  ca.cer -CAkey ca.key  -CAserial ca.srl  -CAcreateserial -in certificate.csr -out certificate.cer
echo -e '\n' >> certificate.cer
cat ca.cer >> certificate.cer


openssl pkcs12 -export -CAfile ca.cer -in certificate.cer -inkey certificate.key -out certificate.pfx

rm -rf *.csr *.srl