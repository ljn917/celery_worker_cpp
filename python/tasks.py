from celery import Celery

app = Celery('tasks', broker='amqp://guest:guest@localhost:5672//', backend='redis://localhost:6379/0')

@app.task
def add(a, b):
    return a + b
