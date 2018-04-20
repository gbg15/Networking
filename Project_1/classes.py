class Node:  

    def __init__(self, dest, cost, n_hop):
        self.dest = dest  
        self.cost = cost
        self.n_hop = n_hop


class RoutingTable(object):

    def __init__(self, name):
        self.vector_list = []
        self.name = name

    def addRoute(self, dest, length, next_hop):
        v = Node(dest, length, next_hop)
        self.vector_list.append(v)

    def deleteRoute(self, dest):
        try:
            v = self.getVector(dest)
            self.vector_list.remove(v)
        except:
            raise ValueError("Route not in the routing table")

    def changeRoute(self, dest, length, next_hop):
        v = self.getVector(dest)
        v.cost = length
        v.n_hop = next_hop

    def contains(self, dest):
        for vector in self.vector_list:
            if vector.dest == dest:
                return vector

    def getVector(self, dest):
        for vector in self.vector_list:
            if vector.dest == dest:
                return vector

    def printRT(self):
        print("Destination  Cost    Next Hop")
        for vector in sorted(self.vector_list, key = lambda vl: int(vl.dest)):
            print("{:^11s} {:^6d} {:^15s}".format(vector.dest, vector.cost, vector.n_hop))


class Router:

    def __init__(self, name):
        self.name = name
        self.rTable = RoutingTable(name)
        self.links = {}
        self.addLink(name, 0)

    def export(self):
        return self.rTable

    def rimport(self, neighborTables):

        returnVal = False
        for neighborTable in neighborTables :                   
            if neighborTable.name in self.links:               
                for vector in neighborTable.vector_list:        
                    vectorCost = vector.cost + self.rTable.getVector(neighborTable.name).cost
                    if self.rTable.contains(vector.dest):
                        if vectorCost < self.rTable.getVector(vector.dest).cost:
                            self.rTable.changeRoute(vector.dest, vectorCost, neighborTable.name)
                            returnVal = True
                    else:
                        self.rTable.addRoute(vector.dest, vectorCost, neighborTable.name)
                        returnVal = True
        return returnVal

    def addLink(self, routerName, cost):
        self.links[routerName] = cost
        self.rTable.addRoute(routerName, cost, routerName)

    def removeLink(self, routerName):
        del self.links[routerName]
        v = self.rTable.getVector(routerName)
        if v.n_hop == routerName:
            self.rTable.deleteRoute(routerName)

    def printR(self):
        self.rTable.printRT()

