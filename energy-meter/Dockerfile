FROM python:3.11-slim

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY fetcher.py .
EXPOSE 8022
CMD uvicorn fetcher:app --host 0.0.0.0 --port 8022
