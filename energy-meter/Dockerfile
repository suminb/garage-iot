FROM python:3.11-slim

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY fetcher.py .
CMD python fetcher.py
