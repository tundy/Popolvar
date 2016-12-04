#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER



#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0

#define MultiToSingle(width, x, y) (width*y + x)
#define SingleToMulti(index, width, x, y) (x = index%width, y = index/width)

// POPOLVAR
#define PATH 'C'
#define SLOW 'H'
#define WALL 'N'
#define DRAK 'D'
#define PRIN 'P'
#define PRIN1 ('P' + 1)
#define PRIN2 ('P' + 2)
#define PRIN3 ('P' + 3)
#define GENE 'G'

#define isTeleport(c) ((c) >= '0' && (c) <= '9')

typedef struct point
{
	int x;
	int y;
} Point;

typedef struct teleport
{
	Point point;
	struct teleport* next;
} Teleport;

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

static Point Start, Drak, Princezna1, Princezna2, Princezna3, Generator;
static Path StartDrak, StartGenerator, GeneratorDrak, DrakGenerator;
static Path DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV;
static Path DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ;
static Path GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3;
static Path P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ;
static Path P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN;
static Path P1G, P2G, P3G;

// A structure to represent a node in adjacency list
typedef struct AdjListNode
{
	Point point;
	int weight;
	struct AdjListNode* next;
} Nodes;

// A structure to represent an adjacency liat
typedef struct AdjList
{
	Nodes* head; // pointer to head node of list
} Graph;


// A utility function to create a new adjacency list node
Nodes* newAdjListNode(char** mapa, int x, int y)
{
	Nodes* newNode = (Nodes*)malloc(sizeof(Nodes));
	newNode->point.x = x;
	newNode->point.y = y;
	newNode->weight = (mapa[y][x] == SLOW) ? 2 : 1;
	newNode->next = NULL;
	return newNode;
}

// A utility function that creates a graph of V vertices
Graph* createGraph(int height, int width)
{
	return (Graph*)malloc(width * height * sizeof(Graph));
}

// Adds an edge to an undirected graph
void addEdge(Graph* graph, int width, int srcX, int srcY, char** mapa, int destX, int destY)
{
	if (mapa[destY][destX] == WALL) return;

	// Add an edge from src to dest.  A new node is added to the adjacency
	// list of src.  The node is added at the begining
	int src = MultiToSingle(width, srcX, srcY);
	Nodes* newNode = newAdjListNode(mapa, destX, destY);
	newNode->next = graph[src].head;
	graph[src].head = newNode;

	// Since graph is undirected, add an edge from dest to src also
	/*int dest = MultiToSingle(width, destX, destY);
	newNode = newAdjListNode(mapa, destX, destY);
	newNode->next = graph[dest].head;
	graph[dest].head = newNode;*/
}

// Structure to represent a min heap node
struct MinHeapNode
{
	int v;
	int dist;
};

// Structure to represent a min heap
struct MinHeap
{
	int size; // Number of heap nodes present currently
	int capacity; // Capacity of min heap
	int* pos; // This is needed for decreaseKey()
	struct MinHeapNode** array;
};

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
		struct MinHeapNode* smallestNode = minHeap->array[smallest];
		struct MinHeapNode* idxNode = minHeap->array[idx];

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
int isInMinHeap(struct MinHeap* minHeap, int v)
{
	if (minHeap->pos[v] < minHeap->size)
		return TRUE;
	return FALSE;
}

typedef struct dijkstraResult
{
	int* time;
	int* back;
} dResult;

// The main function that calulates distances of shortest paths from src to all
// vertices. It is a O(ELogV) function
dResult* dijkstra(Graph* graph, int height, int width, int startX, int startY)
{
	int src = MultiToSingle(width, startX, startY);
	int V = width * height;// Get the number of vertices in graph
	dResult* result = malloc(sizeof(dResult));
	result->time = (int*)malloc(V * sizeof(int)); // dist values used to pick minimum weight edge in cut
	result->back = (int*)malloc(V * sizeof(int)); // dist values used to pick minimum weight edge in cut

	// minHeap represents set E
	struct MinHeap* minHeap = createMinHeap(V);

	// Initialize min heap with all vertices. dist value of all vertices 
	int v;
	for (v = 0; v < V; ++v)
	{
		result->time[v] = INT_MAX ;
		minHeap->array[v] = newMinHeapNode(v, result->time[v]);
		minHeap->pos[v] = v;
	}

	// Make dist value of src vertex as 0 so that it is extracted first
	minHeap->array[src] = newMinHeapNode(src, result->time[src]);
	minHeap->pos[src] = src;
	result->time[src] = 0;
	decreaseKey(minHeap, src, result->time[src]);

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
		Nodes* pCrawl = graph[u].head;
		while (pCrawl != NULL)
		{
			int v = MultiToSingle(width, pCrawl->point.x, pCrawl->point.y);

			// If shortest distance to v is not finalized yet, and distance to v
			// through u is less than its previously calculated distance
			if (isInMinHeap(minHeap, v) && result->time[u] != INT_MAX &&
				pCrawl->weight + result->time[u] < result->time[v])
			{
				result->time[v] = result->time[u] + pCrawl->weight;
				result->back[v] = u;

				// update distance value in min heap also
				decreaseKey(minHeap, v, result->time[v]);
			}
			pCrawl = pCrawl->next;
		}
	}

	return result;
}

void UDLR(char** mapa, int height, int width, int x, int y, Graph* graph)
{
	if (mapa[y][x] == WALL) return;
	if ((x - 1) >= 0)
		addEdge(graph, width, x, y, mapa, x - 1, y);
	if ((y - 1) >= 0)
		addEdge(graph, width, x, y, mapa, x, y - 1);
	if ((x + 1) < width)
		addEdge(graph, width, x, y, mapa, x + 1, y);
	if ((y + 1) < height)
		addEdge(graph, width, x, y, mapa, x, y + 1);
}

int InitRescue(char** mapa, int height, int width, Teleport** teleporty, Graph* graph)
{
	int x, y, temp = 1;

	Generator.x = -1;

	P1G.cesta = P2G.cesta = P3G.cesta =
		DrakPrincenza1GV.cesta = DrakPrincenza2GV.cesta = DrakPrincenza3GV.cesta =
		DrakPrincenza1GZ.cesta = DrakPrincenza2GZ.cesta = DrakPrincenza3GZ.cesta =
		GeneratorPrincenza1.cesta = GeneratorPrincenza2.cesta = GeneratorPrincenza3.cesta =
		P1P2GZ.cesta = P1P3GZ.cesta = P2P1GZ.cesta = P2P3GZ.cesta = P3P1GZ.cesta = P3P2GZ.cesta =
		P1P2GN.cesta = P1P3GN.cesta = P2P1GN.cesta = P2P3GN.cesta = P3P1GN.cesta = P3P2GN.cesta =
		DrakGenerator.cesta = StartGenerator.cesta = GeneratorDrak.cesta = StartDrak.cesta = NULL;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			if (mapa[y][x] == PRIN)
			{
				mapa[y][x] += temp++;
				switch (mapa[y][x])
				{
				case PRIN1:
					Princezna1.x = x;
					Princezna1.y = y;
					break;
				case PRIN2:
					Princezna2.x = x;
					Princezna2.y = y;
					break;
				case PRIN3:
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

			int src = MultiToSingle(width, x, y);
			graph[src].head = NULL;
			UDLR(mapa, height, width, x, y, graph);
		}
	}
	return TRUE;
}

void addTeleports(Teleport** teleporty, Graph* graph, int width)
{
	int i, src;
	Teleport* tSrc;
	Teleport* tDest;
	Nodes* newNode;
	for (i = 0; i < 10; i++)
	{
		tSrc = teleporty[i];
		while (tSrc)
		{
			tDest = teleporty[i];
			while (tDest)
			{
				if (tSrc->point.x != tDest->point.x || tSrc->point.y != tDest->point.y)
				{
					src = MultiToSingle(width, tSrc->point.x, tSrc->point.y);
					newNode = (Nodes*)malloc(sizeof(Nodes));
					newNode->point.x = tDest->point.x;
					newNode->point.y = tDest->point.y;
					newNode->weight = 0;
					newNode->next = graph[src].head;
					graph[src].head = newNode;
				}
				tDest = tDest->next;
			}
			tSrc = tSrc->next;
		}
	}
}

int vytvorCestu(Point start, Point ciel, dResult* dResult, int width, Path* pathBack)
{
	int index, x, y;
	int dest = MultiToSingle(width, ciel.x, ciel.y);
	int src = MultiToSingle(width, start.x, start.y);

	if (dResult->time[dest] != INT_MAX)
	{
		index = dest;
		pathBack->steps = 0;
		pathBack->time = dResult->time[dest];
		while (src != index)
		{
			index = dResult->back[index];
			++pathBack->steps;
		}

		if ((pathBack->cesta = malloc(pathBack->steps * 2 * sizeof(int))) == NULL) return FALSE;
		int index = pathBack->steps * 2;
		while (index > 0)
		{
			SingleToMulti(dest, width, x, y);
			pathBack->cesta[--index] = y;
			pathBack->cesta[--index] = x;
			dest = dResult->back[dest];
		}
	}

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

#define TOSTRING(x) #x

void VypisCesty()
{
	vypisCestu(StartDrak, TOSTRING(StartDrak));
	vypisCestu(StartGenerator, TOSTRING(StartGenerator));

	vypisCestu(DrakGenerator, TOSTRING(DrakGenerator));
	vypisCestu(GeneratorDrak, TOSTRING(GeneratorDrak));

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
		!updateList(list, 5, &StartGenerator, &GeneratorDrak, &DrakPrincenza1GZ, &P1P2GZ, &P2P3GZ) ||
		!updateList(list, 5, &StartGenerator, &GeneratorDrak, &DrakPrincenza1GZ, &P1P3GZ, &P3P2GZ) ||

		!updateList(list, 5, &StartGenerator, &GeneratorDrak, &DrakPrincenza2GZ, &P2P1GZ, &P1P3GZ) ||
		!updateList(list, 5, &StartGenerator, &GeneratorDrak, &DrakPrincenza2GZ, &P2P3GZ, &P3P1GZ) ||

		!updateList(list, 5, &StartGenerator, &GeneratorDrak, &DrakPrincenza3GZ, &P3P1GZ, &P1P2GZ) ||
		!updateList(list, 5, &StartGenerator, &GeneratorDrak, &DrakPrincenza3GZ, &P3P2GZ, &P2P1GZ)
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


int* zachran_princezne(char** mapa, int height, int width, int t, int* dlzka_cesty)
{
	*dlzka_cesty = 0;

	Start.x = 0;
	Start.y = 0;

	Graph* graph;
	dResult* dResult;
	Teleport** teleporty;

	if ((teleporty = (Teleport**)calloc(10, sizeof(Teleport*))) == NULL) return NULL;
	if ((graph = createGraph(height, width)) == NULL) return NULL;
	if (!InitRescue(mapa, height, width, teleporty, graph)) return NULL;

	dResult = dijkstra(graph, height, width, 0, 0);
	int i = MultiToSingle(width, Drak.x, Drak.y);
	if (dResult->time[i] <= t)
		vytvorCestu(Start, Drak, dResult, width, &StartDrak);
	i = MultiToSingle(width, Generator.x, Generator.y);
	if (Generator.x != -1 && dResult->time[i] <= t)
		vytvorCestu(Start, Generator, dResult, width, &StartGenerator);

	dResult = dijkstra(graph, height, width, Drak.x, Drak.y);
	if (Generator.x != -1)
		vytvorCestu(Drak, Generator, dResult, width, &DrakGenerator);
	vytvorCestu(Drak, Princezna1, dResult, width, &DrakPrincenza1GV);
	vytvorCestu(Drak, Princezna2, dResult, width, &DrakPrincenza2GV);
	vytvorCestu(Drak, Princezna3, dResult, width, &DrakPrincenza3GV);
	free(dResult);

	dResult = dijkstra(graph, height, width, Princezna1.x, Princezna1.y);
	if (Generator.x != -1)
		vytvorCestu(Princezna1, Generator, dResult, width, &P1G);
	vytvorCestu(Princezna1, Princezna2, dResult, width, &P1P2GN);
	vytvorCestu(Princezna1, Princezna3, dResult, width, &P1P3GN);
	free(dResult);

	dResult = dijkstra(graph, height, width, Princezna2.x, Princezna2.y);
	if (Generator.x != -1)
		vytvorCestu(Princezna2, Generator, dResult, width, &P2G);
	vytvorCestu(Princezna2, Princezna1, dResult, width, &P2P1GN);
	vytvorCestu(Princezna2, Princezna3, dResult, width, &P2P3GN);
	free(dResult);

	dResult = dijkstra(graph, height, width, Princezna3.x, Princezna3.y);
	if (Generator.x != -1)
		vytvorCestu(Princezna3, Generator, dResult, width, &P3G);
	vytvorCestu(Princezna3, Princezna1, dResult, width, &P3P1GN);
	vytvorCestu(Princezna3, Princezna2, dResult, width, &P3P2GN);
	free(dResult);

	if (Generator.x != -1)
	{
		addTeleports(teleporty, graph, width);

		dResult = dijkstra(graph, height, width, Generator.x, Generator.y);
		i = MultiToSingle(width, Generator.x, Generator.y);
		if (StartGenerator.cesta && (StartGenerator.time + dResult->time[i] <= t))
			vytvorCestu(Generator, Drak, dResult, width, &GeneratorDrak);
		vytvorCestu(Generator, Princezna1, dResult, width, &GeneratorPrincenza1);
		vytvorCestu(Generator, Princezna2, dResult, width, &GeneratorPrincenza2);
		vytvorCestu(Generator, Princezna3, dResult, width, &GeneratorPrincenza3);
		free(dResult);

		dResult = dijkstra(graph, height, width, Drak.x, Drak.y);
		vytvorCestu(Drak, Princezna1, dResult, width, &DrakPrincenza1GZ);
		vytvorCestu(Drak, Princezna2, dResult, width, &DrakPrincenza2GZ);
		vytvorCestu(Drak, Princezna3, dResult, width, &DrakPrincenza3GZ);
		free(dResult);

		dResult = dijkstra(graph, height, width, Princezna1.x, Princezna1.y);
		vytvorCestu(Princezna1, Princezna2, dResult, width, &P1P2GZ);
		vytvorCestu(Princezna1, Princezna3, dResult, width, &P1P3GZ);
		free(dResult);

		dResult = dijkstra(graph, height, width, Princezna2.x, Princezna2.y);
		vytvorCestu(Princezna2, Princezna1, dResult, width, &P2P1GZ);
		vytvorCestu(Princezna2, Princezna3, dResult, width, &P2P3GZ);
		free(dResult);

		dResult = dijkstra(graph, height, width, Princezna3.x, Princezna3.y);
		vytvorCestu(Princezna3, Princezna1, dResult, width, &P3P1GZ);
		vytvorCestu(Princezna3, Princezna2, dResult, width, &P3P2GZ);
		free(dResult);
	}
	//VypisCesty();

	if (StartGenerator.cesta)
	{
		// Existuje cesta Start->Drak->Generator v case t?
		if (DrakGenerator.cesta)
		{
			if (StartGenerator.time + GeneratorDrak.time > DrakGenerator.time + StartDrak.time)
			{
				free(StartGenerator.cesta);
				StartGenerator.cesta = NULL;
				free(GeneratorDrak.cesta);
				GeneratorDrak.cesta = NULL;
				//printf("# Aktivovat generator po drakovi je rychlejsie ako pred drakom\n");
			}
		}
	}


	PathList list;
	list.time = INT_MAX ;
	list.parts = NULL;

	// Start->Generator->Drak->Princezna->Princezna->Princezna
	if (GeneratorDrak.cesta)
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

	int* result;

	if (list.parts)
	{
		*dlzka_cesty = list.steps + 1;
		if ((result = malloc(list.steps * 2 * sizeof(int))) == NULL) return NULL;

		PathPart* part = list.parts;
		int offset = *dlzka_cesty * 2;

		while (part)
		{
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

	strncpy(mapa[i++], "HHHH.HH...H", m);
	strncpy(mapa[i++], "H.HH.....H.", m);
	strncpy(mapa[i++], "HPH...HHHH.", m);
	strncpy(mapa[i++], ".HH.HH.H...", m);
	strncpy(mapa[i++], "H.H.....HH.", m);
	strncpy(mapa[i++], "H.H...HHH.H", m);
	strncpy(mapa[i++], "HHHH.HHH..H", m);
	strncpy(mapa[i++], "HH.H.PHH..P", m);
	strncpy(mapa[i++], "HHH.H.H....", m);
	strncpy(mapa[i++], "..D..H.H...", m);
	strncpy(mapa[i], ".....H...HH", m);

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
