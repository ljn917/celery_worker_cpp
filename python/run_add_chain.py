import tasks
add_chain = (tasks.add.s(10) | tasks.add.s(20) | tasks.add.s(30))
res = add_chain.delay(4)
print(res.get())
print(res.parent.parent.graph)
