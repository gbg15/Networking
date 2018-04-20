from sys import argv
import classes
import sys
import argparse
from _ast import arg


class Simulator:

	def __init__(self, fileName, rounds):
		self.routers = []
		self.fileData = [line.split() for line in open(file, 'r')]
		self.fileName = fileName
		self.rounds = rounds
		for info in self.fileData:
			if self.containsRouter(info[0]) == False:
				new_router = classes.Router(info[0])
				self.routers.append(new_router)
			if self.containsRouter(info[1]) == False:
				new_router = classes.Router(info[1])
				self.routers.append(new_router)
			for router in self.routers:
				if router.name == info[0]:
					router.addLink(info[1], int(info[2]))
				elif router.name == info[1]:
					router.addLink(info[0], int(info[2]))

	def runSim(self):
		for i in range(int(self.rounds)):
			not_converged = True
			count = 1
			while not_converged:
				not_converged = False
				routerTables = self.collectTables()
				for router in self.routers:
					if router.rimport(routerTables) == True:
						print("\n" + "Iteration " + str(count))
						self.printRouters()
						not_converged = True
						count+=1

	def collectTables(self):
		routerTables = []
		for router in self.routers:
			routerTables.append(router.export())
		return routerTables

	def containsRouter(self, name):
		for router in self.routers:
			if router.name == name:
				return True
		return False

	def getRouter(self, name):
		for router in self.routers:
			if router.name == name:
				return router
		return None

	def cost(self, x, y):
		router = self.getRouter(x)
		if router.export().contains(y):
			return int(router.export().getVector(y).cost)
		else:
			return int('inf')

	def printRouters(self):
		for router in sorted(self.routers, key=lambda rtr: int(rtr.name)):
			print("Router: "+router.name)
			router.printR()

if __name__ == '__main__':
	if len(argv) > 2:
		file = argv[1]
		rounds = argv[2]
	else:
		print("Input should be: [textfile.txt] <amount of rounds>")
		exit(-1)
	simulator = Simulator(file, rounds)

	simulator.runSim()
