#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#define TOSTRING(x) #x

#define startX 0
#define startY 0

#define true 1
#define false 0
#define bool int
#define TRUE 1
#define FALSE 0
#define ON TRUE
#define OFF FALSE

#define PATH 'C'
#define SLOW 'H'
#define WALL 'N'
#define DRAK 'D'
#define PRIN 'P'
#define P1 (PRIN + 1)
#define P2 (PRIN + 2)
#define P3 (PRIN + 3)
#define GENE 'G'

#define isTeleport(c) ((c) >= '0' && (c) <= '9')


typedef struct point
{
	int x;
	int y;
} Point;


// A structure to represent a node in adjacency list
struct AdjListNode
{
	int dest;
	int weight;
	struct AdjListNode* next;
};

// A structure to represent an adjacency liat
struct AdjList
{
	struct AdjListNode *head;  // pointer to head node of list
};

// A structure to represent a graph. A graph is an array of adjacency lists.
// Size of array will be V (number of vertices in graph)
struct Graph
{
	int V;
	struct AdjList* array;
};

// A utility function to create a new adjacency list node
struct AdjListNode* newAdjListNode(int dest, int weight)
{
	struct AdjListNode* newNode =
		(struct AdjListNode*) malloc(sizeof(struct AdjListNode));
	newNode->dest = dest;
	newNode->weight = weight;
	newNode->next = NULL;
	return newNode;
}

// A utility function that creates a graph of V vertices
struct Graph* createGraph(int V)
{
	struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph));
	graph->V = V;

	// Create an array of adjacency lists.  Size of array will be V
	graph->array = (struct AdjList*) malloc(V * sizeof(struct AdjList));

	// Initialize each adjacency list as empty by making head as NULL
	for (int i = 0; i < V; ++i)
		graph->array[i].head = NULL;

	return graph;
}

// Adds an edge to an undirected graph
void addEdge(struct Graph* graph, int src, int dest, int weight)
{
	// Add an edge from src to dest.  A new node is added to the adjacency
	// list of src.  The node is added at the begining
	struct AdjListNode* newNode = newAdjListNode(dest, weight);
	newNode->next = graph->array[src].head;
	graph->array[src].head = newNode;

	// Since graph is undirected, add an edge from dest to src also
	newNode = newAdjListNode(src, weight);
	newNode->next = graph->array[dest].head;
	graph->array[dest].head = newNode;
}

// Structure to represent a min heap node
typedef struct MinHeapNode
{
	int  v;
	int dist;
} MinHeapNode;

// Structure to represent a min heap
typedef struct MinHeap
{
	int size;      // Number of heap nodes present currently
	int capacity;  // Capacity of min heap
	int *pos;     // This is needed for decreaseKey()
	struct MinHeapNode **array;
} MinHeap;

// A utility function to create a new Min Heap Node
struct MinHeapNode* newMinHeapNode(int v, int dist)
{
	struct MinHeapNode* minHeapNode =
		(struct MinHeapNode*) malloc(sizeof(struct MinHeapNode));
	minHeapNode->v = v;
	minHeapNode->dist = dist;
	return minHeapNode;
}

// A utility function to create a Min Heap
struct MinHeap* createMinHeap(int capacity)
{
	struct MinHeap* minHeap =
		(struct MinHeap*) malloc(sizeof(struct MinHeap));
	minHeap->pos = (int *)malloc(capacity * sizeof(int));
	minHeap->size = 0;
	minHeap->capacity = capacity;
	minHeap->array =
		(struct MinHeapNode**) malloc(capacity * sizeof(struct MinHeapNode*));
	return minHeap;
}

// A utility function to swap two nodes of min heap. Needed for min heapify
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b)
{
	struct MinHeapNode* t = *a;
	*a = *b;
	*b = t;
}

// A standard function to heapify at given idx
// This function also updates position of nodes when they are swapped.
// Position is needed for decreaseKey()
void minHeapify(struct MinHeap* minHeap, int idx)
{
	int smallest, left, right;
	smallest = idx;
	left = 2 * idx + 1;
	right = 2 * idx + 2;

	if (left < minHeap->size &&
		minHeap->array[left]->dist < minHeap->array[smallest]->dist)
		smallest = left;

	if (right < minHeap->size &&
		minHeap->array[right]->dist < minHeap->array[smallest]->dist)
		smallest = right;

	if (smallest != idx)
	{
		// The nodes to be swapped in min heap
		MinHeapNode *smallestNode = minHeap->array[smallest];
		MinHeapNode *idxNode = minHeap->array[idx];

		// Swap positions
		minHeap->pos[smallestNode->v] = idx;
		minHeap->pos[idxNode->v] = smallest;

		// Swap nodes
		swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);

		minHeapify(minHeap, smallest);
	}
}

// A utility function to check if the given minHeap is ampty or not
int isEmpty(struct MinHeap* minHeap)
{
	return minHeap->size == 0;
}

// Standard function to extract minimum node from heap
struct MinHeapNode* extractMin(struct MinHeap* minHeap)
{
	if (isEmpty(minHeap))
		return NULL;

	// Store the root node
	struct MinHeapNode* root = minHeap->array[0];

	// Replace root node with last node
	struct MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
	minHeap->array[0] = lastNode;

	// Update position of last node
	minHeap->pos[root->v] = minHeap->size - 1;
	minHeap->pos[lastNode->v] = 0;

	// Reduce heap size and heapify root
	--minHeap->size;
	minHeapify(minHeap, 0);

	return root;
}

// Function to decreasy dist value of a given vertex v. This function
// uses pos[] of min heap to get the current index of node in min heap
void decreaseKey(struct MinHeap* minHeap, int v, int dist)
{
	// Get the index of v in  heap array
	int i = minHeap->pos[v];

	// Get the node and update its dist value
	minHeap->array[i]->dist = dist;

	// Travel up while the complete tree is not hepified.
	// This is a O(Logn) loop
	while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist)
	{
		// Swap this node with its parent
		minHeap->pos[minHeap->array[i]->v] = (i - 1) / 2;
		minHeap->pos[minHeap->array[(i - 1) / 2]->v] = i;
		swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);

		// move to parent index
		i = (i - 1) / 2;
	}
}

// A utility function to check if a given vertex
// 'v' is in min heap or not
bool isInMinHeap(struct MinHeap *minHeap, int v)
{
	if (minHeap->pos[v] < minHeap->size)
		return true;
	return false;
}

// A utility function used to print the solution
void printArr(int dist[], int n)
{
	printf("Vertex   Distance from Source\n");
	for (int i = 0; i < n; ++i)
		printf("%d \t\t %d\n", i, dist[i]);
}

// The main function that calulates distances of shortest paths from src to all
// vertices. It is a O(ELogV) function
void dijkstra(struct Graph* graph, int src)
{
	int V = graph->V;// Get the number of vertices in graph
	int *dist = malloc(V * sizeof(int));      // dist values used to pick minimum weight edge in cut
	int *back = malloc(V * sizeof(Point));      // dist values used to pick minimum weight edge in cut

					  // minHeap represents set E
	struct MinHeap* minHeap = createMinHeap(V);

	// Initialize min heap with all vertices. dist value of all vertices 
	for (int v = 0; v < V; ++v)
	{
		dist[v] = INT_MAX;
		minHeap->array[v] = newMinHeapNode(v, dist[v]);
		minHeap->pos[v] = v;
	}

	// Make dist value of src vertex as 0 so that it is extracted first
	minHeap->array[src] = newMinHeapNode(src, dist[src]);
	minHeap->pos[src] = src;
	dist[src] = 0;
	decreaseKey(minHeap, src, dist[src]);

	// Initially size of min heap is equal to V
	minHeap->size = V;

	// In the followin loop, min heap contains all nodes
	// whose shortest distance is not yet finalized.
	while (!isEmpty(minHeap))
	{
		// Extract the vertex with minimum distance value
		struct MinHeapNode* minHeapNode = extractMin(minHeap);
		int u = minHeapNode->v; // Store the extracted vertex number

								// Traverse through all adjacent vertices of u (the extracted
								// vertex) and update their distance values
		struct AdjListNode* pCrawl = graph->array[u].head;
		while (pCrawl != NULL)
		{
			int v = pCrawl->dest;

			// If shortest distance to v is not finalized yet, and distance to v
			// through u is less than its previously calculated distance
			if (isInMinHeap(minHeap, v) && dist[u] != INT_MAX &&
				pCrawl->weight + dist[u] < dist[v])
			{
				dist[v] = dist[u] + pCrawl->weight;

				// update distance value in min heap also
				decreaseKey(minHeap, v, dist[v]);
			}
			pCrawl = pCrawl->next;
		}
	}

	// print the calculated shortest distances
	printArr(dist, V);
}

typedef struct pathBack
{
	int steps;
	int time;
	int* cesta;
} Path;

typedef struct pathPart
{
	Path* part;
	struct pathPart* next;
} PathPart;

typedef struct pathList
{
	int time;
	int steps;
	PathPart* parts;
} PathList;

typedef struct qv
{
	int slowed;
	Point point;
	Point* back;
	int generator;
} QV;

typedef struct teleport
{
	Point point;
	struct teleport* next;
} Teleport;

typedef struct title
{
	int steps;
	int time;
	Point* back;
} Title;

Point Drak, Princezna1, Princezna2, Princezna3, Generator;
Path StartDrak, StartGeneratorDrak, DrakGenerator;
Path DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV;
Path DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ;
Path GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3;
Path P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ;
Path P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN;
Path P1G, P2G, P3G;

void clear(Title** dist, int n, int m)
{
	int x, y;
	for (y = 0; y < n; y++)
		for (x = 0; x < m; x++)
			dist[y][x].time = INT_MAX;

	dist[Drak.y][Drak.x].steps = -1;
	dist[Princezna1.y][Princezna1.x].steps = -1;
	dist[Princezna2.y][Princezna2.x].steps = -1;
	dist[Princezna3.y][Princezna3.x].steps = -1;
	if (Generator.x != -1)
		dist[Generator.y][Generator.x].steps = -1;
}

QV* newQV(QV* back, int y, int x, int slowed)
{
	if (back == NULL) return NULL;

	QV* new_qv;
	if ((new_qv = malloc(sizeof(QV))) == NULL) return NULL;

	new_qv->back = (Point*)&back->point;
	new_qv->point.y = y;
	new_qv->point.x = x;
	new_qv->generator = back->generator;
	new_qv->slowed = slowed;

	return new_qv;
}

int UDLR(int n, int m, Queue* queue, QV* value, Point point)
{
	if (point.y - 1 >= 0)
		if (!enqueue(queue, newQV(value, point.y - 1, point.x, 0)))
			return FALSE;
	if (point.x - 1 >= 0)
		if (!enqueue(queue, newQV(value, point.y, point.x - 1, 0)))
			return FALSE;
	if (point.y + 1 < n)
		if (!enqueue(queue, newQV(value, point.y + 1, point.x, 0)))
			return FALSE;
	if (point.x + 1 < m)
		if (!enqueue(queue, newQV(value, point.y, point.x + 1, 0)))
			return FALSE;
	return TRUE;
}

#define STEPS(point) dist[point.y][point.x].steps
#define TIME(point) dist[point.y][point.x].time
#define MAP(point) mapa[point.y][point.x]
#define _STEPS(point) dist[point->y][point->x].steps
#define _TIME(point) dist[point->y][point->x].time

void updateDist(Title** dist, QV* value, int time)
{
	TIME(value->point) = time;
	STEPS(value->point) = _STEPS(value->back) + 1;
	dist[value->point.y][value->point.x].back = value->back;
}

QV* newStart(Title** dist, int x, int y, int gen)
{
	QV* value;
	if ((value = malloc(sizeof(QV))) == NULL) return NULL;

	value->point.x = x;
	value->point.y = y;
	value->back = &value->point;
	value->generator = gen;
	value->slowed = 0;
	dist[y][x].time = 0;
	dist[y][x].steps = 0;
	return value;
}

int addToQueue(char** mapa, int n, int m, Teleport** teleporty, Queue* queue, QV* value)
{
	if (value->generator && isTeleport(MAP(value->point)))
	{
		Teleport* teleport = teleporty[MAP(value->point) - '0'];
		while (teleport)
		{
			if (teleport->point.x != value->point.x || teleport->point.y != value->point.y)
				if (!enqueue(queue, newQV(value, teleport->point.y, teleport->point.x, -1)))
					return FALSE;
			teleport = teleport->next;
		}
	}
	return UDLR(n, m, queue, value, value->point);
}
/*
int dijkstra(char** mapa, int n, int m, Teleport** teleporty, Queue* queue, Title** dist, int maxTime)
{
	while (any(queue) && (STEPS(Drak) == -1 || STEPS(Princezna1) == -1 || STEPS(Princezna2) == -1 || STEPS(Princezna3) == -1 || (Generator.x == -1 || STEPS(Generator) == -1)))
	{
		QV* value = top(queue);
		pop(queue);

		if (MAP(value->point) == WALL)
			continue;

		int time = _TIME(value->back);
		if (value->slowed != -1)
		{
			if (!value->slowed++ && MAP(value->point) == SLOW)
			{
				if (!enqueue(queue, value))
					return FALSE;
				continue;
			}

			time += MAP(value->point) == SLOW ? 2 : 1;
		}

		if (time < 0 || time > maxTime)
			continue;

		if (time < TIME(value->point))
		{
			if (dist[value->point.y][value->point.x].time != INT_MAX)
				continue;
			updateDist(dist, value, time);
			if (!addToQueue(mapa, n, m, teleporty, queue, value))
				return FALSE;
		}
	}
	clearQueue(queue);
	return TRUE;
}
*/

int vytvorCestu(Point ciel, Title** dist, Path* pathBack)
{
	if (STEPS(ciel) >= 0)
	{
		pathBack->steps = STEPS(ciel);
		pathBack->time = TIME(ciel);
		if ((pathBack->cesta = malloc(pathBack->steps * 2 * sizeof(int))) == NULL) return FALSE;
		int index = pathBack->steps * 2;
		Point* p = (Point*)&ciel;
		while (index > 0)
		{
			pathBack->cesta[--index] = p->y;
			pathBack->cesta[--index] = p->x;
			p = dist[p->y][p->x].back;
		}
	}
	return TRUE;
}

int vytvorStartDrak(int t, Point Drak, Point Generator, Title** dist, Title** distGen, Path* startDrak, Path* startGeneratorDrak)
{
	if (Generator.x != -1 && distGen[Drak.y][Drak.x].steps >= 0)
	{
		int index, offset = 0;
		if (STEPS(Generator) > 0)
		{
			startGeneratorDrak->steps = distGen[Drak.y][Drak.x].steps + STEPS(Generator);
			startGeneratorDrak->time = distGen[Drak.y][Drak.x].time + TIME(Generator);
			offset = STEPS(Generator) * 2;
		}
		else if (Generator.x != startX || Generator.y != startY)
		{
			return TRUE;
		}
		else
		{
			startGeneratorDrak->steps = distGen[Drak.y][Drak.x].steps;
			startGeneratorDrak->time = distGen[Drak.y][Drak.x].time;
		}

		// Ak sa k Drakovi pomocou generatora nedostanes ani v case t nema zmysel si to pamatat
		if (startGeneratorDrak->time <= t)
		{
			if ((startGeneratorDrak->cesta = malloc(startGeneratorDrak->steps * 2 * sizeof(int))) == NULL) return FALSE;
			index = distGen[Drak.y][Drak.x].steps * 2;
			Point* p = (Point*)&Drak;
			while (index > 0)
			{
				startGeneratorDrak->cesta[offset + --index] = p->y;
				startGeneratorDrak->cesta[offset + --index] = p->x;
				p = distGen[p->y][p->x].back;
			}
			while (offset > 0)
			{
				startGeneratorDrak->cesta[--offset] = p->y;
				startGeneratorDrak->cesta[--offset] = p->x;
				p = dist[p->y][p->x].back;
			}
		}
		//else printf("# Nedokazem vcas dobehnut k drakovi pomocou generatora\n");
	}
	//else printf("# Nedokazem vcas dobehnut k drakovi pomocou generatora\n");

	// Ak cesta cez generator bola rychlejsie nema zmysel si pamatat cestu bez generatora
	if (STEPS(Drak) >= 0)
	{
		if (TIME(Drak) <= t)
		{
			if (startGeneratorDrak->cesta == NULL || STEPS(Drak) < startGeneratorDrak->steps)
			{
				startDrak->steps = dist[Drak.y][Drak.x].steps;
				startDrak->time = dist[Drak.y][Drak.x].time;
				if ((startDrak->cesta = malloc(startDrak->steps * 2 * sizeof(int))) == NULL) return FALSE;
				int index = STEPS(Drak) * 2;
				Point* p = (Point*)&Drak;
				while (index > 0)
				{
					startDrak->cesta[--index] = p->y;
					startDrak->cesta[--index] = p->x;
					p = dist[p->y][p->x].back;
				}
			}
			//else printf("# Pomocou generatora dokazem dobehnut k drakovi rychlejsie\n");
		}
		//else printf("# Nedokazem vcas dobehnut k drakovi bez pomoci generatora\n");
	}
	//else printf("# Nedokazem sa dostat k drakovi bez generatora\n");

	return TRUE;
}

void vypisCestu(Path pathBack, char* cesta)
{
	if (pathBack.cesta != NULL)
	{
		printf("%s v case %d po %d polickach\n", cesta, pathBack.time, pathBack.steps);
		int i;
		for (i = 0; i < pathBack.steps; ++i)
			printf("[%d;%d] ", pathBack.cesta[i * 2], pathBack.cesta[i * 2 + 1]);
		putchar('\n');
	}
}

int updateList(PathList* list, int n_args, ...)
{
	int i, time = 0, steps = 0;
	va_list ap;
	Path* part;
	PathPart *partList = NULL, *temp;

	va_start(ap, n_args);
	for (i = 1; i <= n_args; i++)
	{
		part = va_arg(ap, Path*);

		if (part->cesta == NULL)
		{
			va_end(ap);
			return TRUE;
		}

		if ((temp = malloc(sizeof(PathPart))) == NULL) return FALSE;
		temp->next = partList;
		temp->part = part;
		partList = temp;

		time += part->time;
		steps += part->steps;

		if (time >= list->time)
		{
			va_end(ap);
			return TRUE;
		}
	}
	va_end(ap);

	//if (list->parts == NULL || time < list->time)
	{
		list->time = time;
		list->steps = steps;
		while (list->parts)
		{
			temp = list->parts;
			list->parts = temp->next;
			free(temp);
		}
		list->parts = partList;
	}

	return TRUE;
}

int InitRescue(char** mapa, int n, int m, Teleport** teleporty)
{
	int x, y, temp = 1;

	P1G.cesta = P2G.cesta = P3G.cesta =
		DrakPrincenza1GV.cesta = DrakPrincenza2GV.cesta = DrakPrincenza3GV.cesta =
		DrakPrincenza1GZ.cesta = DrakPrincenza2GZ.cesta = DrakPrincenza3GZ.cesta =
		GeneratorPrincenza1.cesta = GeneratorPrincenza2.cesta = GeneratorPrincenza3.cesta =
		P1P2GZ.cesta = P1P3GZ.cesta = P2P1GZ.cesta = P2P3GZ.cesta = P3P1GZ.cesta = P3P2GZ.cesta =
		P1P2GN.cesta = P1P3GN.cesta = P2P1GN.cesta = P2P3GN.cesta = P3P1GN.cesta = P3P2GN.cesta =
		DrakGenerator.cesta = StartGeneratorDrak.cesta = StartDrak.cesta = NULL;

	for (y = 0; y < n; y++)
	{
		for (x = 0; x < m; x++)
		{
			//if (Drak.x != -1 && Princezna1.x != -1 && Princezna2.x != -1 && Princezna3.x != -1 && Generator.x != -1) break;

			if (mapa[y][x] == PRIN)
			{
				mapa[y][x] += temp++;
				switch (mapa[y][x])
				{
				case P1:
					Princezna1.x = x;
					Princezna1.y = y;
					break;
				case P2:
					Princezna2.x = x;
					Princezna2.y = y;
					break;
				case P3:
					Princezna3.x = x;
					Princezna3.y = y;
					break;
				default: break;
				}
			}
			else if (mapa[y][x] == DRAK)
			{
				Drak.x = x;
				Drak.y = y;
			}
			else if (mapa[y][x] == GENE)
			{
				Generator.x = x;
				Generator.y = y;
			}
			else if (isTeleport(mapa[y][x]))
			{
				int index = mapa[y][x] - '0';
				Teleport* teleport;
				if ((teleport = malloc(sizeof(Teleport))) == NULL) return FALSE;
				teleport->next = teleporty[index];
				teleport->point.x = x;
				teleport->point.y = y;
				teleporty[index] = teleport;
			}
		}
	}
	return TRUE;
}

void fasterfrom2(Point point1, Point point2, Path* path1, Path* path2, Title** dist)
{
	if (TIME(point1) == TIME(point2))
	{
		vytvorCestu(point1, dist, path1);
		vytvorCestu(point2, dist, path2);
	}
	else if (TIME(point1) < TIME(point2))
		vytvorCestu(point1, dist, path1);
	else
		vytvorCestu(point2, dist, path2);
}

void fasterfrom3(Point point1, Point point2, Point point3, Path* path1, Path* path2, Path* path3, Title** dist)
{
	if (TIME(point1) == TIME(point2))
	{
		if (TIME(point1) == TIME(point3))
		{
			// point1 == point2 == point3
			vytvorCestu(point1, dist, path1);
			vytvorCestu(point2, dist, path2);
			vytvorCestu(point3, dist, path3);
		}
		else if (TIME(point3) < TIME(point1))
		{
			// point3 < point1 && point3 < point2
			vytvorCestu(point3, dist, path3);
		}
		else
		{
			// point1 < point3 && point2 < point3 && point1 == point2
			vytvorCestu(point1, dist, path1);
			vytvorCestu(point2, dist, path2);
		}
	}
	else if (TIME(point1) < TIME(point2))
		fasterfrom2(point1, point3, path1, path3, dist);
	else
		fasterfrom2(point2, point3, path2, path3, dist);
}

int sdppp(PathList* list)
{
	return !(
		!updateList(list, 4, &StartDrak, &DrakPrincenza1GV, &P1P2GN, &P2P3GN) ||
		!updateList(list, 4, &StartDrak, &DrakPrincenza1GV, &P1P3GN, &P3P2GN) ||

		!updateList(list, 4, &StartDrak, &DrakPrincenza2GV, &P2P1GN, &P1P3GN) ||
		!updateList(list, 4, &StartDrak, &DrakPrincenza2GV, &P2P3GN, &P3P1GN) ||

		!updateList(list, 4, &StartDrak, &DrakPrincenza3GV, &P3P1GN, &P1P2GN) ||
		!updateList(list, 4, &StartDrak, &DrakPrincenza3GV, &P3P2GN, &P2P1GN)
	);
}

int sgdppp(PathList* list)
{
	return !(
		!updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza1GZ, &P1P2GZ, &P2P3GZ) ||
		!updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza1GZ, &P1P3GZ, &P3P2GZ) ||

		!updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza2GZ, &P2P1GZ, &P1P3GZ) ||
		!updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza2GZ, &P2P3GZ, &P3P1GZ) ||

		!updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza3GZ, &P3P1GZ, &P1P2GZ) ||
		!updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza3GZ, &P3P2GZ, &P2P1GZ)
	);
}

int sdgppp(PathList* list)
{
	return !(
		!updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza1, &P1P2GZ, &P2P3GZ) ||
		!updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza1, &P1P3GZ, &P3P1GZ) ||

		!updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza2, &P2P1GZ, &P1P3GZ) ||
		!updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza2, &P2P3GZ, &P3P1GZ) ||

		!updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza3, &P3P1GZ, &P1P2GZ) ||
		!updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza3, &P3P2GZ, &P2P1GZ)
	);
}

int sdpgpp(PathList* list)
{
	return !(
		!updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1G, &GeneratorPrincenza2, &P2P3GZ) ||
		!updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1G, &GeneratorPrincenza3, &P3P2GZ) ||

		!updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2G, &GeneratorPrincenza1, &P1P3GZ) ||
		!updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2G, &GeneratorPrincenza3, &P3P1GZ) ||

		!updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3G, &GeneratorPrincenza1, &P1P2GZ) ||
		!updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3G, &GeneratorPrincenza2, &P2P1GZ)
	);
}

int sdppgp(PathList* list)
{
	return !(
		!updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1P2GN, &P2G, &GeneratorPrincenza3) ||
		!updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1P3GN, &P3G, &GeneratorPrincenza2) ||

		!updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2P1GN, &P1G, &GeneratorPrincenza3) ||
		!updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2P3GN, &P3G, &GeneratorPrincenza1) ||

		!updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3P1GN, &P1G, &GeneratorPrincenza2) ||
		!updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3P2GN, &P2G, &GeneratorPrincenza1)
	);
}

int startSearch(char** mapa, int n, int m, Teleport** teleporty, Point startPoint, int gStatus, Queue* queue, Title** dist, int t)
{
	clear(dist, n, m);
	QV* start;
	if ((start = newStart(dist, startPoint.x, startPoint.y, gStatus)) == NULL) return FALSE;
	if (!UDLR(n, m, queue, start, start->point)) return FALSE;
	return dijkstra(mapa, n, m, teleporty, queue, dist, t);
}

void VypisCesty()
{
	vypisCestu(StartDrak, TOSTRING(StartDrak));
	vypisCestu(DrakGenerator, TOSTRING(DrakGenerator));
	vypisCestu(StartGeneratorDrak, TOSTRING(StartGeneratorDrak));

	vypisCestu(DrakPrincenza1GV, TOSTRING(DrakPrincenza1GV));
	vypisCestu(DrakPrincenza2GV, TOSTRING(DrakPrincenza2GV));
	vypisCestu(DrakPrincenza3GV, TOSTRING(DrakPrincenza3GV));

	vypisCestu(DrakPrincenza1GZ, TOSTRING(DrakPrincenza1GZ));
	vypisCestu(DrakPrincenza2GZ, TOSTRING(DrakPrincenza2GZ));
	vypisCestu(DrakPrincenza3GZ, TOSTRING(DrakPrincenza3GZ));

	vypisCestu(GeneratorPrincenza1, TOSTRING(GeneratorPrincenza1));
	vypisCestu(GeneratorPrincenza2, TOSTRING(GeneratorPrincenza2));
	vypisCestu(GeneratorPrincenza3, TOSTRING(GeneratorPrincenza3));

	vypisCestu(P1P2GZ, TOSTRING(P1P2GZ));
	vypisCestu(P1P3GZ, TOSTRING(P1P3GZ));
	vypisCestu(P2P1GZ, TOSTRING(P2P1GZ));
	vypisCestu(P2P3GZ, TOSTRING(P2P3GZ));
	vypisCestu(P3P1GZ, TOSTRING(P3P1GZ));
	vypisCestu(P3P2GZ, TOSTRING(P3P2GZ));

	vypisCestu(P1P2GN, TOSTRING(P1P2GN));
	vypisCestu(P1P3GN, TOSTRING(P1P3GN));
	vypisCestu(P2P1GN, TOSTRING(P2P1GN));
	vypisCestu(P2P3GN, TOSTRING(P2P3GN));
	vypisCestu(P3P1GN, TOSTRING(P3P1GN));
	vypisCestu(P3P2GN, TOSTRING(P3P2GN));

	vypisCestu(P1G, TOSTRING(P1G));
	vypisCestu(P2G, TOSTRING(P2G));
	vypisCestu(P3G, TOSTRING(P3G));
}

int* result = NULL;

int* zachran_princezne(char** mapa, int n, int m, int t, int* dlzka_cesty)
{
	*dlzka_cesty = 0;

#ifdef _MSC_VER
#pragma region Init
#endif
	Generator.x = -1;
	Teleport** teleporty;
	if ((teleporty = calloc(10, sizeof(Teleport*))) == NULL) return NULL;

	// Zisti suradnice
	if (!InitRescue(mapa, n, m, teleporty)) return NULL;
	//printf("# InitRescue ... OK\n");

	//int k;
	//for (k = 0; k < n; k++)
	//	printf("%.*s\n", m, mapa[k]);

	int i;
	QV* start;
	Queue* queue;
	Title** dist;
	Title** distGen;

	if ((queue = newQueue()) == NULL) return NULL;
	if ((dist = malloc(n * sizeof(Title*))) == NULL) return NULL;
	if ((distGen = malloc(n * sizeof(Title*))) == NULL) return NULL;
	for (i = 0; i < n; i++)
	{
		if ((dist[i] = malloc(m * sizeof(Title))) == NULL) return NULL;
		if ((distGen[i] = malloc(m * sizeof(Title))) == NULL) return NULL;
	}
#ifdef _MSC_VER
#pragma endregion
#endif
	printf("# Init ... OK\n");

	// ReSharper disable CppLocalVariableMightNotBeInitialized
	clear(dist, n, m);
	if ((start = newStart(dist, startX, startY, mapa[startY][startX] == GENE)) == NULL) return NULL;
	if (!UDLR(n, m, queue, start, start->point)) return NULL;
	if (!dijkstra(mapa, n, m, teleporty, queue, dist, t)) return NULL;
	//printf("# Dist 1 ... OK\n");

	if (Generator.x != -1)
	{
		if (!startSearch(mapa, n, m, teleporty, Generator, ON, queue, distGen, t)) return NULL;
		//fasterfrom3(Princezna1, Princezna2, Princezna3, &GeneratorPrincenza1, &GeneratorPrincenza2, &GeneratorPrincenza3, distGen);
		if (!vytvorCestu(Princezna1, distGen, &GeneratorPrincenza1)) return NULL;
		if (!vytvorCestu(Princezna2, distGen, &GeneratorPrincenza2)) return NULL;
		if (!vytvorCestu(Princezna3, distGen, &GeneratorPrincenza3)) return NULL;
	}
	//printf("# Dist 2 ... OK\n");
	if (!vytvorStartDrak(t, Drak, Generator, dist, distGen, &StartDrak, &StartGeneratorDrak)) return NULL;
	printf("# Start ... OK\n");
	// PrincezneGeneratorOn
	if (Generator.x != -1)
	{
		if (!startSearch(mapa, n, m, teleporty, Princezna1, ON, queue, dist, INT_MAX)) return NULL;
		//fasterfrom2(Princezna2, Princezna3, &P1P2GZ, &P1P3GZ, dist);
		if (!vytvorCestu(Princezna2, dist, &P1P2GZ)) return NULL;
		if (!vytvorCestu(Princezna3, dist, &P1P3GZ)) return NULL;

		if (!startSearch(mapa, n, m, teleporty, Princezna2, ON, queue, dist, INT_MAX)) return NULL;
		//fasterfrom2(Princezna1, Princezna3, &P2P1GZ, &P2P3GZ, dist);
		if (!vytvorCestu(Princezna1, dist, &P2P1GZ)) return NULL;
		if (!vytvorCestu(Princezna3, dist, &P2P3GZ)) return NULL;

		if (!startSearch(mapa, n, m, teleporty, Princezna3, ON, queue, dist, INT_MAX)) return NULL;
		//fasterfrom2(Princezna1, Princezna2, &P3P1GZ, &P3P2GZ, dist);
		if (!vytvorCestu(Princezna1, dist, &P3P1GZ)) return NULL;
		if (!vytvorCestu(Princezna2, dist, &P3P2GZ)) return NULL;

		if (!startSearch(mapa, n, m, teleporty, Drak, ON, queue, dist, INT_MAX)) return NULL;
		//fasterfrom3(Princezna1, Princezna2, Princezna3, &DrakPrincenza1GZ, &DrakPrincenza2GZ, &DrakPrincenza3GZ, dist);
		if (!vytvorCestu(Princezna1, dist, &DrakPrincenza1GZ)) return NULL;
		if (!vytvorCestu(Princezna2, dist, &DrakPrincenza2GZ)) return NULL;
		if (!vytvorCestu(Princezna3, dist, &DrakPrincenza3GZ)) return NULL;
	}
	printf("# PrincezneGeneratorOn ... OK\n");

	if (StartDrak.cesta) // Cesta k drakovi bez generatora
	{
		// Drak->Princezne bez generatora
		if (!startSearch(mapa, n, m, teleporty, Drak, OFF, queue, dist, INT_MAX)) return NULL;
		if (!vytvorCestu(Princezna1, dist, &DrakPrincenza1GV)) return NULL;
		if (!vytvorCestu(Princezna2, dist, &DrakPrincenza2GV)) return NULL;
		if (!vytvorCestu(Princezna3, dist, &DrakPrincenza3GV)) return NULL;
		if (Generator.x != -1)
			if (!vytvorCestu(Generator, dist, &DrakGenerator)) return NULL;

		if (!startSearch(mapa, n, m, teleporty, Princezna1, OFF, queue, dist, INT_MAX)) return NULL;
		if (!vytvorCestu(Princezna2, dist, &P1P2GN)) return NULL;
		if (!vytvorCestu(Princezna3, dist, &P1P3GN)) return NULL;
		if (Generator.x != -1)
			if (!vytvorCestu(Generator, dist, &P1G)) return NULL;

		if (!startSearch(mapa, n, m, teleporty, Princezna2, OFF, queue, dist, INT_MAX)) return NULL;
		if (!vytvorCestu(Princezna1, dist, &P2P1GN)) return NULL;
		if (!vytvorCestu(Princezna3, dist, &P2P3GN)) return NULL;
		if (Generator.x != -1)
			if (!vytvorCestu(Generator, dist, &P2G)) return NULL;

		if (!startSearch(mapa, n, m, teleporty, Princezna3, OFF, queue, dist, INT_MAX)) return NULL;
		if (!vytvorCestu(Princezna1, dist, &P3P1GN)) return NULL;
		if (!vytvorCestu(Princezna2, dist, &P3P2GN)) return NULL;
		if (Generator.x != -1)
			if (!vytvorCestu(Generator, dist, &P3G)) return NULL;
	}
	// ReSharper restore CppLocalVariableMightNotBeInitialized

	printf("# NoGenerator ... OK\n");
	// Cesta k drakovi s generatorom
	if (StartGeneratorDrak.cesta)
	{
		// Existuje cesta Start->Drak->Generator v case t?
		if (DrakGenerator.cesta)
		{
			if (StartGeneratorDrak.time > DrakGenerator.time + StartDrak.time)
			{
				free(StartGeneratorDrak.cesta);
				StartGeneratorDrak.cesta = NULL;
				//printf("# Aktivovat generator po drakovi je rychlejsie ako pred drakom\n");
			}
		}
	}
	printf("# SGD ... OK\n");

	PathList list;
	list.time = INT_MAX;
	list.parts = NULL;

	// Start->Generator->Drak->Princezna->Princezna->Princezna
	if (StartGeneratorDrak.cesta)
		if (!sgdppp(&list)) return NULL;

	// Start->Drak->...
	if (StartDrak.cesta)
	{
		// Start->Drak->Generator->Princezna->Princezna->Princezna
		if (DrakGenerator.cesta)
			if (!sdgppp(&list)) return NULL;

		// Start->Drak->Princezna->Princezna->Princezna
		if (!sdppp(&list)) return NULL;

		if (Generator.x != -1)
		{
			// Start->Drak->Princezna->Generator->Princezna->Princezna
			if (!sdpgpp(&list)) return NULL;

			// Start->Drak->Princezna->Princezna->Generator->Princezna
			if (!sdppgp(&list)) return NULL;
		}
	}
	printf("# Search ... OK\n");

	//VypisCesty(StartDrak, StartGeneratorDrak, DrakGenerator, DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV, DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ, GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3, P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ, P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN, P1G, P2G, P3G);

	if (list.parts)
	{
		*dlzka_cesty = list.steps + 1;
		if ((result = malloc(list.steps * 2 * sizeof(int))) == NULL) return NULL;

		PathPart* part = list.parts;
		int offset = *dlzka_cesty * 2;

		//printf("Final loop\n");
		//printf("Last pointer should be: %p\n", &StartDrak);
		//vypisCestu(StartDrak, TOSTRING(StartDrak));
		while (part)
		{
			//printf("# pointer: %p | count of values: %d | first value: [%d:%d]\n", part->part, part->part->steps, part->part->cesta[0], part->part->cesta[1]);

			int end = part->part->steps * 2;
			offset -= end;

			if (offset < 0)
				break;

			for (i = 0; i < end; i++)
				result[offset + i] = part->part->cesta[i];

			part = part->next;
		}
		result[0] = 0;
		result[1] = 0;
	}
	else
	{
		result = NULL;
	}
	printf("# Result ... OK\n");


	Teleport* tTemp;
	for (i = 0; i < 10; i++)
	{
		while (teleporty[i])
		{
			tTemp = teleporty[i];
			teleporty[i] = tTemp->next;
			free(tTemp);
		}
	}
	free(teleporty);

	for (i = 0; i < n; i++)
	{
		free(dist[i]);
		free(distGen[i]);
	}
	free(dist);
	free(distGen);

	free(StartDrak.cesta);
	free(StartGeneratorDrak.cesta);
	free(DrakGenerator.cesta);
	free(DrakPrincenza1GV.cesta);
	free(DrakPrincenza2GV.cesta);
	free(DrakPrincenza3GV.cesta);
	free(DrakPrincenza1GZ.cesta);
	free(DrakPrincenza2GZ.cesta);
	free(DrakPrincenza3GZ.cesta);
	free(GeneratorPrincenza1.cesta);
	free(GeneratorPrincenza2.cesta);
	free(GeneratorPrincenza3.cesta);
	free(P1P2GZ.cesta);
	free(P1P3GZ.cesta);
	free(P2P1GZ.cesta);
	free(P2P3GZ.cesta);
	free(P3P1GZ.cesta);
	free(P3P2GZ.cesta);
	free(P1P2GN.cesta);
	free(P1P3GN.cesta);
	free(P2P1GN.cesta);
	free(P2P3GN.cesta);
	free(P3P1GN.cesta);
	free(P3P2GN.cesta);
	free(P1G.cesta);
	free(P2G.cesta);
	free(P3G.cesta);

	printf("# Free ... OK\n");

	//printf("%d\n", *dlzka_cesty);
	//for (i = 0; i < *dlzka_cesty; ++i)
	//	printf("%d %d\n", result[i * 2], result[i * 2 + 1]);

	return result;
}

#ifdef _MSC_VER

void main()
{
	int n = 11;
	int m = 11;
	char** mapa = malloc(n * sizeof(char*));
	int i;
	for (i = 0; i < n; i++)
		mapa[i] = malloc(m * sizeof(char));

	i = 0;
	/*strncpy(mapa[i++], "HHHHHCHCCCHHHCHHHHPDHCCCCHCCCH", m);
	strncpy(mapa[i++], "CCHCCCCCCHCCCCCHCHHHCCCCCCHCCH", m);
	strncpy(mapa[i++], "CCCCCHPCCCHHCCHCCCCHCCCCHCCCCH", m);
	strncpy(mapa[i++], "CCCCHHHHCHCCCHCCCCCHCCHCCCCHCH", m);
	strncpy(mapa[i], "CPHHCCHHCCCHHCHHHCCHCHCHHCCHCC", m);*/

	strncpy(mapa[i++], "HHHHCHHCCCH", m);
	strncpy(mapa[i++], "HCHHCCCCCHC", m);
	strncpy(mapa[i++], "HPHCCCHHHHC", m);
	strncpy(mapa[i++], "CHHCHHCHCCC", m);
	strncpy(mapa[i++], "HCHCCCCCHHC", m);
	strncpy(mapa[i++], "HCHCCCHHHCH", m);
	strncpy(mapa[i++], "HHHHCHHHCCH", m);
	strncpy(mapa[i++], "HHCHCPHHCCP", m);
	strncpy(mapa[i++], "HHHCHCHCCCC", m);
	strncpy(mapa[i++], "CCDCCHCHCCC", m);
	strncpy(mapa[i], "CCCCCHCCCHH", m);

	/*strncpy(mapa[i++], "CCCCCCCCHGCCCHCCCCCCHCCCCCCCHCCCHHHCHCCCCCCCCCCCCC", m);
	strncpy(mapa[i++], "CCCCCCCHC2CHCCCHCHCCCCCHHHCHCCHHHCCC0CCCCHCCHCCCCC", m);
	strncpy(mapa[i++], "CCCHCCCC0CCC2CCCCHCCCCHCCCCCCCCCHCCCCCCHCCCCCHHCHC", m);
	strncpy(mapa[i++], "CHCCCCCCCCCCCCCCCCCHHHCCCCHCHCHCHCC3CCHCCCCHCCCHCC", m);
	strncpy(mapa[i++], "CCCHCCCHCHHCCHCCCCCHHCCHCCHCCCCCCHHCCHCCHHCHCCCCCH", m);
	strncpy(mapa[i++], "HCCCCHCCHCCC0CCCCCCCCHCHCCCCCHCCCCHCCCCCCHHCCHCCCC", m);
	strncpy(mapa[i++], "CCCHCHCCCCCHHCCCCCCHCCCCCHCCPCCCCCCCHCCCCCCCCHCHCC", m);
	strncpy(mapa[i++], "CCCHCHCCHCHCCCCHHCCCCCCCCCCCHCCHCCCCCCCCCCCHCCCHHC", m);
	strncpy(mapa[i++], "CCCCCCCCCCCCCHCCCCCHCHHCCHCCCCCCHHCCC3CHCHHCCCCCCH", m);
	strncpy(mapa[i++], "CCCCCCHCCHC0HCCHCCHHCCCHCCCCCCCCHCCCCCCCCCCHHC3HCC", m);
	strncpy(mapa[i++], "CCCHCCCCCCHCCHCCCCHHCHCCCCCCCCCHCCHCCCCCHCCCCCCCCC", m);
	strncpy(mapa[i++], "HCCCCHHCCCCCCCCCCCCCHCCCCHHCCCCCCHCCHCCCCCCHCCCHCC", m);
	strncpy(mapa[i++], "CCCHCCCCCCHCCCCCHHCCC0CCCHCHH1CCCCCCHCCCCCHCCHCCCC", m);
	strncpy(mapa[i++], "HHCCCCCCHHCCCHHCHCCCCCCCCCCCCCCCCCCCHCHCCHCCCCHCCC", m);
	strncpy(mapa[i++], "CCCCHCHCCHHCHHCCCHHCCHHCCCHCCCCCCCHHCCCCCCCCHCCCHC", m);
	strncpy(mapa[i++], "CCCHCCHHCHCCHHCCCCCHCCCCCCCCCHCCCCCCCCCCCCCCCCCCCC", m);
	strncpy(mapa[i++], "CCCCCCC2CHCCHCCCCCHCCCHCCCHHCCHCCHHCCCCCCCHCC0HHHC", m);
	strncpy(mapa[i++], "HHCCCPCCCHCHH1CCCHCHCCCHCCCHCHHCCCCCCCCCCCCHCCCCCC", m);
	strncpy(mapa[i++], "CCCCCCCHCHCCCHHCHCCHCHCCCCCCCCCCHCCCCCCCCCCCCHCCCC", m);
	strncpy(mapa[i++], "CCCHCCCCCCCHHCH0CCCCCCCHCHCCCCHHHCCCCHCHCCCCCCC2CC", m);
	strncpy(mapa[i++], "CCHHCCHCCCCCCCCCCCCCCCHCCCCCCCCHCCCCCCHCCCCCHCHCHH", m);
	strncpy(mapa[i++], "HHCHCCCCCCCCCCCCCCCHCHCCCCCCHCCHCCCCCCCCCCCCCCCHCC", m);
	strncpy(mapa[i++], "CCHCC0CHCCCCCHCCCCCCCCCCCHCHCCCC3HHCCCCCCC2CCCCCCC", m);
	strncpy(mapa[i++], "HHCCCCHCCHCCCCCCCHCCCCCCCCCCCHCCHHCCHCCCHCHCHCHHCC", m);
	strncpy(mapa[i++], "CCHCCCCHCCCCCCCCHCCCCCCCCCCCCHHHHHCHCCCCCCCCHCCCCC", m);
	strncpy(mapa[i++], "CCCCCCCCHHCCCCCCCCCCCCHCCCHHCCCCCCHDCCCCCHCHCCHCCC", m);
	strncpy(mapa[i++], "HCCCCCCCHHCCHCHCCCCHCHCCCCCCCCCCCCCHCCC2CCCCCCCCCC", m);
	strncpy(mapa[i++], "HCHCHCHHHCCCCHHCCCCCHCC1CHCCHHCHHCCHCCCCCCCCCCCCCC", m);
	strncpy(mapa[i++], "CCCCCCCCCCCHCCCCCCHCCCHCCHHHCCCCCHCCCCCHCCCCCCCCHC", m);
	strncpy(mapa[i++], "CCHCHCCCCCCCCHCCCCHCHC2HHCCPHC0HCHHHCC2HCCHHHHCCCC", m);
	strncpy(mapa[i++], "CCCCCCCCCCCHCCCCHCHCCCCCCCCHCCCCCCCCHHHCCHCCHCCCCH", m);
	strncpy(mapa[i++], "CCHCCCCCCCCCCCCCHCCCCCCCCCCCCCHCCCCCCCHCCCCHCCHHHC", m);
	strncpy(mapa[i++], "CHCHCCCCCCCHCCHHHCCHCCCCCHCHCCCCCHCHCCHCCCCCCCHCCC", m);
	strncpy(mapa[i++], "HCCHCCHHCCCCCCCHHHCHHHCHHCCHCHCCCCCCCHCCCCCHCCCCCC", m);
	strncpy(mapa[i++], "CCCCCCCCCCCCCCCCHCCCCCCHCHCCCHCCHHHCCHCHCCHHCCCCCH", m);
	strncpy(mapa[i++], "CHCCHCCCHCCCCHCCCCCHHHCHCCCHHCCCCCCCHHCCCHCHCCHCCC", m);
	strncpy(mapa[i++], "CCHCCCHCCCCCCCCCCC2CCCCCCHCCCCCCHCCCCCCCHCCCHCCCCC", m);
	strncpy(mapa[i++], "HCCCCHHHCCCCCCHCCCHCHCCCCHCCCHCCCCCHCCCCCCCCCCCHCH", m);
	strncpy(mapa[i++], "CHHHHHHHCCHCCCCCHCCCHCCCCCCCCHCHCCCCCCCHHCCCCHCCC0", m);
	strncpy(mapa[i], "CCCCHHCCCHHCHCCCCCHHCCCHCCCCCCHHCCCCCCCHCCCHCCCCCC", m);*/

	/*strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "....0.....", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "....0.....", m);
	strncpy(mapa[i++], "........P.", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".P........", m);
	strncpy(mapa[i++], "..D3......", m);
	strncpy(mapa[i++], ".0........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "0.........", m);
	strncpy(mapa[i++], ".......H..", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "........H.", m);
	strncpy(mapa[i++], ".......0..", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "...H...2..", m);
	strncpy(mapa[i++], "...1......", m);
	strncpy(mapa[i++], "G.........", m);
	strncpy(mapa[i++], ".0........", m);
	strncpy(mapa[i++], "........2.", m);
	strncpy(mapa[i++], "...H......", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "2.........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "...2......", m);
	strncpy(mapa[i++], "2.........", m);
	strncpy(mapa[i++], "...P......", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..3.......", m);
	strncpy(mapa[i++], ".H........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "0......H..", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".3........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "...3......", m);
	strncpy(mapa[i++], "........1.", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".....3....", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "........3.", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".2...1....", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "....3.....", m);
	strncpy(mapa[i++], "..3.......", m);
	strncpy(mapa[i++], ".......2..", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".....0....", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".H........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".....0....", m);
	strncpy(mapa[i++], ".....H....", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "......2...", m);
	strncpy(mapa[i++], "2.........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "1...0.....", m);
	strncpy(mapa[i++], "...2..3...", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".....0....", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i++], ".......0..", m);
	strncpy(mapa[i++], "..........", m);
	strncpy(mapa[i], "..........", m);*/

	/*for (i = 0; i < 0; i++)
	strncpy(mapa[i], "....................", m);

	strncpy(mapa[i++], "....................", m);
	strncpy(mapa[i++], "......N.D.0.........", m);
	strncpy(mapa[i++], ".....1N........P....", m);
	strncpy(mapa[i++], "..H.................", m);
	strncpy(mapa[i++], "...............P....", m);
	strncpy(mapa[i++], "..G.0...............", m);
	strncpy(mapa[i++], "...............P....", m);
	strncpy(mapa[i++], "....................", m);
	strncpy(mapa[i++], "...............1....", m);
	strncpy(mapa[i++], "....................", m);

	for (; i < n; i++)
	strncpy(mapa[i], "....................", m);*/

	int dlzka_cesty;
	int* cesta = zachran_princezne(mapa, n, m, 66, &dlzka_cesty);

	printf("Zachranit vsetky princezne dokazem v %d kokoch\n", dlzka_cesty);
	for (i = 0; i < dlzka_cesty; ++i)
		printf("%d %d\n", cesta[i * 2], cesta[i * 2 + 1]);

	for (i = 0; i < n; i++)
		free(mapa[i]);
	free(mapa);

	getchar();
}

#endif
