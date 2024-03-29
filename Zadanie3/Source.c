#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define TOSTRING(x) #x

#define startX 0
#define startY 0

#define TRUE 1
#define FALSE 0
#define ON TRUE
#define OFF FALSE

#define path 'C'
#define slow 'H'
#define wall 'N'
#define drak 'D'
#define prin 'P'
#define p1 (prin + 1)
#define p2 (prin + 2)
#define p3 (prin + 3)
#define gene 'G'

#define isTeleport(c) ((c) >= '0' && (c) <= '9')

#ifdef _MSC_VER
#pragma region Queue<void*>
#endif

typedef struct queueValue
{
	void* value;
	struct queueValue* next;
} qValue;

typedef struct queue
{
	qValue* first;
	qValue* last;
} Queue;

#define newQueue() (Queue*)calloc(1, sizeof(Queue))

void enqueue(Queue* queue, void* title)
{
	if (queue->first)
	{
		queue->last->next = (qValue*)malloc(sizeof(qValue));
		queue->last = queue->last->next;
	}
	else
	{
		queue->last = queue->first = (qValue*)malloc(sizeof(qValue));
	}
	queue->last->value = title;
	queue->last->next = NULL;
}

/*void * dequeue(Queue* queue)
{
	if (queue->first)
	{
		qValue* temp = queue->first;
		queue->first = queue->first->next;
		void * value = temp->value;
		free(temp);
		return value;
	}
	return NULL;
}*/

void* top(const Queue* queue)
{
	if (queue->first)
		return queue->first->value;
	return NULL;
}

void pop(Queue* queue)
{
	if (queue->first)
	{
		qValue* temp = queue->first;
		queue->first = queue->first->next;
		free(temp);
	}
}

int any(const Queue* queue)
{
	return queue->first != NULL;
}

void clearQueue(Queue* queue)
{
	while (queue->first)
	{
		qValue* temp = queue->first;
		queue->first = queue->first->next;
		free(temp);
	}
}

#ifdef _MSC_VER
#pragma endregion
#endif

typedef struct point
{
	int x;
	int y;
} Point;

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

static void clear(Title** dist, const int n, const int m, const Point point1, const Point point2, const Point point3, const Point point4, const Point gp)
{
	int x, y;
	for (y = 0; y < n; y++)
	{
		for (x = 0; x < m; x++)
		{
			dist[y][x].time = INT_MAX;
			//dist[y][x].back = NULL;
			//dist[y][x].steps = -1;
		}
	}

	dist[point1.y][point1.x].steps = -1;
	dist[point2.y][point2.x].steps = -1;
	dist[point3.y][point3.x].steps = -1;
	dist[point4.y][point4.x].steps = -1;
	if (gp.x != -1)
		dist[gp.y][gp.x].steps = -1;
}

static QV* newQV(const QV* back, const int y, const int x)
{
	QV* new_qv = malloc(sizeof(QV));
	new_qv->back = (Point*)&back->point;
	new_qv->point.y = y;
	new_qv->point.x = x;
	new_qv->generator = back->generator;
	new_qv->slowed = 0;
	return new_qv;
}

static void UDLR(const int n, const int m, Queue* queue, QV* value, const Point point)
{
	if (point.y - 1 >= 0)
		enqueue(queue, newQV(value, point.y - 1, point.x));
	if (point.x - 1 >= 0)
		enqueue(queue, newQV(value, point.y, point.x - 1));
	if (point.y + 1 < n)
		enqueue(queue, newQV(value, point.y + 1, point.x));
	if (point.x + 1 < m)
		enqueue(queue, newQV(value, point.y, point.x + 1));
}

#define STEPS(point) dist[point.y][point.x].steps
#define TIME(point) dist[point.y][point.x].time
#define MAP(point) mapa[point.y][point.x]
#define _STEPS(point) dist[point->y][point->x].steps
#define _TIME(point) dist[point->y][point->x].time
//#define _MAP(point) mapa[point->y][point->x]

static void updateDist(Title** dist, const QV* value, const int time)
{
	TIME(value->point) = time;
	STEPS(value->point) = _STEPS(value->back) + 1;
	dist[value->point.y][value->point.x].back = value->back;
}

static QV* newStart(Title** dist, const int x, const int y, const int gen)
{
	QV* value = malloc(sizeof(QV));
	value->point.x = x;
	value->point.y = y;
	value->back = &value->point;
	value->generator = gen;
	value->slowed = 0;
	dist[y][x].time = 0;
	dist[y][x].steps = 0;
	return value;
}

static void addToQueue(const char** mapa, const int n, const int m, const Teleport** teleporty, Queue* queue, QV* value)
{
	if (value->generator && isTeleport(MAP(value->point)))
	{
		Teleport* teleport = (Teleport*)teleporty[MAP(value->point) - '0'];
		while (teleport)
		{
			UDLR(n, m, queue, value, teleport->point);
			teleport = teleport->next;
		}
	}
	else
	{
		UDLR(n, m, queue, value, value->point);
	}
}

static void dijkstra(const char** mapa, const int n, const int m, Teleport** teleporty, Queue* queue, Title** dist, const int maxTime, const Point point1, const Point point2, const Point point3, const Point point4, const Point gp)
{
	while (any(queue) && (STEPS(point1) == -1 || STEPS(point2) == -1 || STEPS(point3) == -1 || STEPS(point4) == -1 || (gp.x == -1 || STEPS(gp) == -1)))
	{
		QV* value = top(queue);
		pop(queue);

		if (MAP(value->point) == wall)
			continue;

		if (!value->slowed++ && MAP(value->point) == slow)
		{
			enqueue(queue, value);
			continue;
		}

		int time = MAP(value->point) == slow ? 2 : 1;
		time += _TIME(value->back);

		if (time < 0 || time > maxTime)
			continue;

		if (time < TIME(value->point))
		{
			if (TIME(value->point) != INT_MAX)
			{
				printf("# ???\n");
			}
			updateDist(dist, value, time);
			addToQueue(mapa, n, m, teleporty, queue, value);
		}
	}
	clearQueue(queue);
}

static void vytvorCestu(const Point ciel, Title** dist, Path* pathBack)
{
	if (STEPS(ciel) >= 0)
	{
		pathBack->steps = STEPS(ciel);
		pathBack->time = TIME(ciel);
		pathBack->cesta = malloc(pathBack->steps * 2 * sizeof(int));
		int index = pathBack->steps * 2;
		Point* p = (Point*)&ciel;
		while (index > 0)
		{
			pathBack->cesta[--index] = p->x;
			pathBack->cesta[--index] = p->y;
			p = dist[p->y][p->x].back;
		}
	}
}

static void vytvorStartDrak(int t, const Point Drak, const Point Generator, Title** dist, Title** distGen, Path* startDrak, Path* startGeneratorDrak)
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
			return;
		}
		else
		{
			startGeneratorDrak->steps = distGen[Drak.y][Drak.x].steps;
			startGeneratorDrak->time = distGen[Drak.y][Drak.x].time;
		}

		// Ak sa k Drakovi pomocou generatora nedostanes ani v case t nema zmysel si to pamatat
		if (startGeneratorDrak->time <= t)
		{
			startGeneratorDrak->cesta = malloc(startGeneratorDrak->steps * 2 * sizeof(int));
			index = distGen[Drak.y][Drak.x].steps * 2;
			Point* p = (Point*)&Drak;
			while (index > 0)
			{
				startGeneratorDrak->cesta[offset + --index] = p->x;
				startGeneratorDrak->cesta[offset + --index] = p->y;
				p = distGen[p->y][p->x].back;
			}
			while (offset > 0)
			{
				startGeneratorDrak->cesta[--offset] = p->x;
				startGeneratorDrak->cesta[--offset] = p->y;
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
				startDrak->cesta = malloc(startDrak->steps * 2 * sizeof(int));
				int index = STEPS(Drak) * 2;
				Point* p = (Point*)&Drak;
				while (index > 0)
				{
					startDrak->cesta[--index] = p->x;
					startDrak->cesta[--index] = p->y;
					p = dist[p->y][p->x].back;
				}
			}
			//else printf("# Pomocou generatora dokazem dobehnut k drakovi rychlejsie\n");
		}
		//else printf("# Nedokazem vcas dobehnut k drakovi bez pomoci generatora\n");
	}
	//else printf("# Nedokazem sa dostat k drakovi bez generatora\n");
}

static void vypisCestu(const Path pathBack, const char* cesta)
{
	if (pathBack.cesta != NULL)
	{
		printf("%s v case %d po %d polickach\n", cesta, pathBack.time, pathBack.steps);
		/*int i;
		for (i = 0; i < pathBack.steps; ++i)
			printf("[%d;%d] ", pathBack.cesta[i * 2], pathBack.cesta[i * 2 + 1]);
		putchar('\n');*/
	}
}

static void updateList(PathList* list, const int n_args, ...)
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
			return;
		}

		temp = malloc(sizeof(PathPart));
		temp->next = partList;
		temp->part = part;
		partList = temp;;

		time += part->time;
		steps += part->steps;

		if (time >= list->time)
		{
			va_end(ap);
			return;
		}
	}
	va_end(ap);

	//if (list->parts == NULL || time < list->time)
	{
		list->time = time;
		list->steps = steps;
		list->parts = partList;
	}
}

static void InitRescue(char** mapa, const int n, const int m, Point* Drak, Point* Princezna1, Point* Princezna2, Point* Princezna3, Point* Generator, Teleport** teleporty)
{
	int x, y, temp = 1;
	for (y = 0; y < n; y++)
	{
		for (x = 0; x < m; x++)
		{
			//if (Drak.x != -1 && Princezna1.x != -1 && Princezna2.x != -1 && Princezna3.x != -1 && Generator.x != -1) break;

			if (mapa[y][x] == prin)
			{
				mapa[y][x] += temp++;
				switch (mapa[y][x])
				{
				case p1:
					Princezna1->x = x;
					Princezna1->y = y;
					break;
				case p2:
					Princezna2->x = x;
					Princezna2->y = y;
					break;
				case p3:
					Princezna3->x = x;
					Princezna3->y = y;
					break;
				default: break;
				}
			}
			else if (mapa[y][x] == drak)
			{
				Drak->x = x;
				Drak->y = y;
			}
			else if (mapa[y][x] == gene)
			{
				Generator->x = x;
				Generator->y = y;
			}
			else if (isTeleport(mapa[y][x]))
			{
				int index = mapa[y][x] - '0';
				Teleport* teleport = malloc(sizeof(Teleport));
				teleport->next = teleporty[index];
				teleport->point.x = x;
				teleport->point.y = y;
				teleporty[index] = teleport;
			}
		}
	}
}

static void fasterfrom2(const Point point1, const Point point2, Path* path1, Path* path2, const Title** dist)
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

static void fasterfrom3(const Point point1, const Point point2, const Point point3, Path* path1, Path* path2, Path* path3, const Title** dist)
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

static void sdppp(const Path StartDrak, const Path DrakPrincenza1GV, const Path DrakPrincenza2GV, const Path DrakPrincenza3GV, const Path P1P2GN, const Path P1P3GN, const Path P2P1GN, const Path P2P3GN, const Path P3P1GN, const Path P3P2GN, PathList* list)
{
	updateList(list, 4, &StartDrak, &DrakPrincenza1GV, &P1P2GN, &P2P3GN);
	updateList(list, 4, &StartDrak, &DrakPrincenza1GV, &P1P3GN, &P3P2GN);

	updateList(list, 4, &StartDrak, &DrakPrincenza2GV, &P2P1GN, &P1P3GN);
	updateList(list, 4, &StartDrak, &DrakPrincenza2GV, &P2P3GN, &P3P1GN);

	updateList(list, 4, &StartDrak, &DrakPrincenza3GV, &P3P1GN, &P1P2GN);
	updateList(list, 4, &StartDrak, &DrakPrincenza3GV, &P3P2GN, &P2P1GN);
}

static void sgdppp(const Path StartGeneratorDrak, const Path DrakPrincenza1GZ, const Path DrakPrincenza2GZ, const Path DrakPrincenza3GZ, const Path P1P2GZ, const Path P1P3GZ, const Path P2P1GZ, const Path P2P3GZ, const Path P3P1GZ, const Path P3P2GZ, PathList* list)
{
	updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza1GZ, &P1P2GZ, &P2P3GZ);
	updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza1GZ, &P1P3GZ, &P3P2GZ);

	updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza2GZ, &P2P1GZ, &P1P3GZ);
	updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza2GZ, &P2P3GZ, &P3P1GZ);

	updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza3GZ, &P3P1GZ, &P1P2GZ);
	updateList(list, 4, &StartGeneratorDrak, &DrakPrincenza3GZ, &P3P2GZ, &P2P1GZ);
}

static void sdgppp(const Path StartDrak, const Path DrakGenerator, const Path GeneratorPrincenza1, const Path GeneratorPrincenza2, const Path GeneratorPrincenza3, const Path P1P2GZ, const Path P1P3GZ, const Path P2P1GZ, const Path P2P3GZ, const Path P3P1GZ, const Path P3P2GZ, PathList* list)
{
	updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza1, &P1P2GZ, &P2P3GZ);
	updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza1, &P1P3GZ, &P3P1GZ);

	updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza2, &P2P1GZ, &P1P3GZ);
	updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza2, &P2P3GZ, &P3P1GZ);

	updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza3, &P3P1GZ, &P1P2GZ);
	updateList(list, 5, &StartDrak, &DrakGenerator, &GeneratorPrincenza3, &P3P2GZ, &P2P1GZ);
}

static void sdpgpp(const Path StartDrak, const Path DrakPrincenza1GV, const Path DrakPrincenza2GV, const Path DrakPrincenza3GV, const Path GeneratorPrincenza1, const Path GeneratorPrincenza2, const Path GeneratorPrincenza3, const Path P1P2GZ, const Path P1P3GZ, const Path P2P1GZ, const Path P2P3GZ, const Path P3P2GZ, const Path P1G, const Path P2G, const Path P3G, PathList* list)
{
	updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1G, &GeneratorPrincenza2, &P2P3GZ);
	updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1G, &GeneratorPrincenza3, &P3P2GZ);

	updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2G, &GeneratorPrincenza1, &P1P3GZ);
	updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2G, &GeneratorPrincenza3, &P3P2GZ);

	updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3G, &GeneratorPrincenza1, &P1P2GZ);
	updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3G, &GeneratorPrincenza2, &P2P1GZ);
}

static void sdppgp(const Path StartDrak, const Path DrakPrincenza1GV, const Path DrakPrincenza2GV, const Path DrakPrincenza3GV, const Path GeneratorPrincenza1, const Path GeneratorPrincenza2, const Path GeneratorPrincenza3, const Path P1P2GN, const Path P1P3GN, const Path P2P1GN, const Path P2P3GN, const Path P3P1GN, const Path P3P2GN, const Path P1G, const Path P2G, const Path P3G, PathList* list)
{
	updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1P2GN, &P2G, &GeneratorPrincenza3);
	updateList(list, 5, &StartDrak, &DrakPrincenza1GV, &P1P3GN, &P3G, &GeneratorPrincenza2);

	updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2P1GN, &P1G, &GeneratorPrincenza3);
	updateList(list, 5, &StartDrak, &DrakPrincenza2GV, &P2P3GN, &P3G, &GeneratorPrincenza1);

	updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3P1GN, &P1G, &GeneratorPrincenza2);
	updateList(list, 5, &StartDrak, &DrakPrincenza3GV, &P3P2GN, &P2G, &GeneratorPrincenza1);
}

static void startSearch(const char** mapa, const int n, const int m, const Teleport** teleporty, const Point startPoint, const Point point1, const Point point2, const Point point3, const Point point4, int gStatus, Queue* queue, Title** dist)
{
	clear(dist, n, m, startPoint, point1, point2, point3, point4);
	QV* start = newStart(dist, startPoint.x, startPoint.y, gStatus);
	UDLR(n, m, queue, start, start->point);
	dijkstra(mapa, n, m, teleporty, queue, dist, INT_MAX, startPoint, point1, point2, point3, point4);
}

static void VypisCesty(const Path StartDrak, const Path StartGeneratorDrak, const Path DrakGenerator, const Path DrakPrincenza1GV, const Path DrakPrincenza2GV, const Path DrakPrincenza3GV, const Path DrakPrincenza1GZ, const Path DrakPrincenza2GZ, const Path DrakPrincenza3GZ, const Path GeneratorPrincenza1, const Path GeneratorPrincenza2, const Path GeneratorPrincenza3, const Path P1P2GZ, const Path P1P3GZ, const Path P2P1GZ, const Path P2P3GZ, const Path P3P1GZ, const Path P3P2GZ, const Path P1P2GN, const Path P1P3GN, const Path P2P1GN, const Path P2P3GN, const Path P3P1GN, const Path P3P2GN, const Path P1G, const Path P2G, const Path P3G)
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

int* zachran_princezne(char** mapa, int n, int m, int t, int* dlzka_cesty)
{
#ifdef _MSC_VER
#pragma region Init
#endif
	int i;
	Teleport** teleporty = calloc(10, sizeof(Teleport*));
	Point Drak, Princezna1, Princezna2, Princezna3, Generator;
	/*Drak.x = Princezna1.x = Princezna2.x = Princezna3.x =*/
	Generator.x = -1;

	// Zisti suradnice
	InitRescue(mapa, n, m, &Drak, &Princezna1, &Princezna2, &Princezna3, &Generator, teleporty);

	QV* start;
	Queue* queue = newQueue();
	Path StartDrak, StartGeneratorDrak, DrakGenerator;
	Path DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV;
	Path DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ;
	Path GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3;
	Path P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ;
	Path P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN;
	Path P1G, P2G, P3G;

	Title** dist = malloc(n * sizeof(Title*));
	Title** distGen = malloc(n * sizeof(Title*));
	for (i = 0; i < n; i++)
	{
		dist[i] = malloc(m * sizeof(Title));
		distGen[i] = malloc(m * sizeof(Title));
	}

	P1G.cesta = P2G.cesta = P3G.cesta =
		DrakPrincenza1GV.cesta = DrakPrincenza2GV.cesta = DrakPrincenza3GV.cesta =
		DrakPrincenza1GZ.cesta = DrakPrincenza2GZ.cesta = DrakPrincenza3GZ.cesta =
		GeneratorPrincenza1.cesta = GeneratorPrincenza2.cesta = GeneratorPrincenza3.cesta =
		P1P2GZ.cesta = P1P3GZ.cesta = P2P1GZ.cesta = P2P3GZ.cesta = P3P1GZ.cesta = P3P2GZ.cesta =
		P1P2GN.cesta = P1P3GN.cesta = P2P1GN.cesta = P2P3GN.cesta = P3P1GN.cesta = P3P2GN.cesta =
		DrakGenerator.cesta = StartGeneratorDrak.cesta = StartDrak.cesta = NULL;
#ifdef _MSC_VER
#pragma endregion
#endif


	// ReSharper disable CppLocalVariableMightNotBeInitialized
	clear(dist, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
	start = newStart(dist, startX, startY, mapa[startY][startX] == gene);
	UDLR(n, m, queue, start, start->point);
	dijkstra(mapa, n, m, teleporty, queue, dist, t, Drak, Princezna1, Princezna2, Princezna3, Generator);

	if (Generator.x != -1)
	{
		clear(distGen, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(distGen, Generator.x, Generator.y, ON);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, distGen, t, Drak, Princezna1, Princezna2, Princezna3, Generator);
		fasterfrom3(Princezna1, Princezna2, Princezna3, &GeneratorPrincenza1, &GeneratorPrincenza2, &GeneratorPrincenza3, distGen);
	}
	vytvorStartDrak(t, Drak, Generator, dist, distGen, &StartDrak, &StartGeneratorDrak);

	// PrincezneGeneratorOn
	if (Generator.x != -1)
	{
		startSearch(mapa, n, m, teleporty, Princezna1, Drak, Princezna2, Princezna3, Generator, ON, queue, dist);
		fasterfrom2(Princezna2, Princezna3, &P1P2GZ, &P1P3GZ, dist);

		startSearch(mapa, n, m, teleporty, Princezna2, Princezna1, Drak, Princezna3, Generator, ON, queue, dist);
		fasterfrom2(Princezna1, Princezna3, &P2P1GZ, &P2P3GZ, dist);

		startSearch(mapa, n, m, teleporty, Princezna3, Princezna1, Princezna2, Drak, Generator, ON, queue, dist);
		fasterfrom2(Princezna1, Princezna2, &P3P1GZ, &P3P2GZ, dist);

		startSearch(mapa, n, m, teleporty, Drak, Princezna1, Princezna2, Princezna3, Generator, ON, queue, dist);
		fasterfrom3(Princezna1, Princezna2, Princezna3, &DrakPrincenza1GZ, &DrakPrincenza2GZ, &DrakPrincenza3GZ, dist);
	}

	if (StartDrak.cesta) // Cesta k drakovi bez generatora
	{
		// Drak->Princezne bez generatora
		startSearch(mapa, n, m, teleporty, Drak, Princezna1, Princezna2, Princezna3, Generator, OFF, queue, dist);
		vytvorCestu(Princezna1, dist, &DrakPrincenza1GV);
		vytvorCestu(Princezna2, dist, &DrakPrincenza2GV);
		vytvorCestu(Princezna3, dist, &DrakPrincenza3GV);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &DrakGenerator);

		startSearch(mapa, n, m, teleporty, Princezna1, Drak, Princezna2, Princezna3, Generator, OFF, queue, dist);
		vytvorCestu(Princezna2, dist, &P1P2GN);
		vytvorCestu(Princezna3, dist, &P1P3GN);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &P1G);

		startSearch(mapa, n, m, teleporty, Princezna2, Princezna1, Drak, Princezna3, Generator, OFF, queue, dist);
		vytvorCestu(Princezna1, dist, &P2P1GN);
		vytvorCestu(Princezna3, dist, &P2P3GN);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &P2G);

		startSearch(mapa, n, m, teleporty, Princezna3, Princezna1, Princezna2, Drak, Generator, OFF, queue, dist);
		vytvorCestu(Princezna1, dist, &P3P1GN);
		vytvorCestu(Princezna2, dist, &P3P2GN);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &P3G);
	}
	// ReSharper restore CppLocalVariableMightNotBeInitialized

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

	PathList list;
	list.time = INT_MAX;
	list.parts = NULL;

	// Start->Generator->Drak->Princezna->Princezna->Princezna
	if (StartGeneratorDrak.cesta)
		sgdppp(StartGeneratorDrak, DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ, P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ, &list);

	// Start->Drak->...
	if (StartDrak.cesta)
	{
		// Start->Drak->Generator->Princezna->Princezna->Princezna
		if (DrakGenerator.cesta)
			sdgppp(StartDrak, DrakGenerator, GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3, P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ, &list);

		// Start->Drak->Princezna->Princezna->Princezna
		sdppp(StartDrak, DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV, P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN, &list);

		if (Generator.x != -1)
		{
			// Start->Drak->Princezna->Generator->Princezna->Princezna
			sdpgpp(StartDrak, DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV, GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3, P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P2GZ, P1G, P2G, P3G, &list);

			// Start->Drak->Princezna->Princezna->Generator->Princezna
			sdppgp(StartDrak, DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV, GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3, P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN, P1G, P2G, P3G, &list);
		}
	}

	//VypisCesty(StartDrak, StartGeneratorDrak, DrakGenerator, DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV, DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ, GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3, P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ, P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN, P1G, P2G, P3G);

	int* result;
	if (list.parts)
	{
		*dlzka_cesty = list.steps;
		result = malloc(list.steps * 2 * sizeof(int));

		PathPart* part = list.parts;
		int offset = list.steps * 2;

		while (part)
		{
			int end = part->part->steps * 2;
			offset -= end;

			for (i = 0; i < end; i++)
				result[offset + i] = part->part->cesta[i];

			part = part->next;
		}
	}
	else
	{
		result = NULL;
		*dlzka_cesty = 0;
	}

	free(teleporty);
	for (i = 0; i < n; i++)
	{
		free(dist[i]);
		free(distGen[i]);
	}
	free(dist);
	free(distGen);

	return result;
}

#ifdef _MSC_VER

void main()
{
	const int n = 1000;
	const int m = 20;
	char** mapa = malloc(n * sizeof(char*));
	int i;
	for (i = 0; i < n; i++)
		mapa[i] = malloc(m * sizeof(char));

	for (i = 0; i < 0; i++)
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
		strncpy(mapa[i], "....................", m);

	int dlzka_cesty;
	int* cesta = zachran_princezne(mapa, n, m, 100, &dlzka_cesty);

	printf("Zachranit vsetky princezne dokazem v %d kokoch\n", dlzka_cesty);
	for (i = 0; i < dlzka_cesty; ++i)
		printf("%d %d\n", cesta[i * 2], cesta[i * 2 + 1]);

	for (i = 0; i < n; i++)
		free(mapa[i]);
	free(mapa);

	getchar();
}

#endif
