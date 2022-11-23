
# Notes

## Compiling Arduino file through arduino-cli

```
arduino core install
    esp32:esp32
    esp8266:esp8266 

arduino-cli compile -b esp32:esp32:esp32 /home/jonascsantos/ScanAndFirmwareUpload/ScanAndFirmwareUpload.ino
```

## Docker commands | Azure registry and hosting

docker login indoorpositioning.azurecr.io

docker build -t indoorpositioning.azurecr.io/indoor-positioning:latest .

(To test: docker run -p 80:80 indoorpositioning.azurecr.io/indoor-positioning )

docker push indoorpositioning.azurecr.io/indoor-positioning:latest