FROM python:3.9

WORKDIR /fastapi-app

COPY requirements.txt .

RUN pip install --no-cache-dir --upgrade -r requirements.txt 

COPY ./main.py ./app/main.py

EXPOSE 80

CMD ["python", "./app/main.py"]

# FROM python:3.9

# ADD featureVectorConverter.py .

# RUN pip3 install scikit-learn micromlgen

# CMD ["python", "./featureVectorConverter.py"]