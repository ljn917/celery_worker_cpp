import tasks
res = tasks.add.apply_async((4, 4), link=tasks.add.s(10))
print(res.get())
