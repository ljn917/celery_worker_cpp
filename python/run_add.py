import tasks
res = tasks.add.delay(4, 4)
print(res.get())

# no need to declare the python task
res = tasks.app.send_task('tasks.add',(4, 4))
print(res.get())
