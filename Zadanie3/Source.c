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

Queue* newQueue()
{
	return (Queue*)calloc(1, sizeof(Queue));
}

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

void* top(Queue* queue)
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

int any(Queue* queue)
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

void clear(Title** dist, int n, int m, Point point1, Point point2, Point point3, Point point4, Point gp)
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
	if(gp.x != -1)
		dist[gp.y][gp.x].steps = -1;
}

QV* newQV(QV* value, int y, int x)
{
	QV* new_qv = malloc(sizeof(QV));
	new_qv->back = &value->point;
	new_qv->point.y = y;
	new_qv->point.x = x;
	new_qv->generator = value->generator;
	new_qv->slowed = 0;
	return new_qv;
}

void UDLR(int n, int m, Queue* queue, QV* value, Point point)
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

void updateDist(Title** dist, QV* value, int time)
{
	TIME(value->point) = time;
	STEPS(value->point) = _STEPS(value->back) + 1;
	dist[value->point.y][value->point.x].back = value->back;
}

QV* newStart(Title** dist, int x, int y, int gen)
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

void addToQueue(char** mapa, int n, int m, Teleport** teleporty, Queue* queue, QV* value)
{
	if (value->generator && isTeleport(MAP(value->point)))
	{
		Teleport* teleport = teleporty[MAP(value->point) - '0'];
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

void dijkstra(char** mapa, int n, int m, Teleport** teleporty, Queue* queue, Title** dist, int maxTime, Point point1, Point point2, Point point3, Point point4, Point gp)
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
			if(TIME(value->point) != INT_MAX)
			{
				printf("# ???\n");
			}
			updateDist(dist, value, time);
			addToQueue(mapa, n, m, teleporty, queue, value);
		}
	}
	clearQueue(queue);
}

void vytvorCestu(Point ciel, Title** dist, Path* pathBack)
{
	if (STEPS(ciel) >= 0)
	{
		pathBack->steps = STEPS(ciel);
		pathBack->time = TIME(ciel);
		pathBack->cesta = malloc(pathBack->steps * 2 * sizeof(int));
		int index = pathBack->steps * 2;
		Point* p = &ciel;
		while (index > 0)
		{
			pathBack->cesta[--index] = p->x;
			pathBack->cesta[--index] = p->y;
			p = dist[p->y][p->x].back;
		}
	}
}

void vytvorStartDrak(int t, Point Drak, Point Generator, Title** dist, Title** distGen, Path* startDrak, Path* startGeneratorDrak)
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
			Point* p = &Drak;
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
				Point* p = &Drak;
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

void vypisCestu(const Path pathBack, const char* cesta)
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

void updateList(PathList* list, int n_args, ...)
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

int* zachran_princezne(char** mapa, int n, int m, int t, int* dlzka_cesty)
{
#ifdef _MSC_VER
#pragma region Init
#endif
	int x, y;
	Point Drak, Princezna1, Princezna2, Princezna3, Generator;
	Generator.x = -1;
	Teleport** teleporty = calloc(10, sizeof(Teleport*));

	int temp = 1;
	for (y = 0; y < n; y++)
	{
		for (x = 0; x < m; x++)
		{
			if (mapa[y][x] == prin)
			{
				mapa[y][x] += temp++;
				switch (mapa[y][x])
				{
				case p1:
					Princezna1.x = x;
					Princezna1.y = y;
					break;
				case p2:
					Princezna2.x = x;
					Princezna2.y = y;
					break;
				case p3:
					Princezna3.x = x;
					Princezna3.y = y;
					break;
				default: break;
				}
			}
			else if (mapa[y][x] == drak)
			{
				Drak.x = x;
				Drak.y = y;
			}
			else if (mapa[y][x] == gene)
			{
				Generator.x = x;
				Generator.y = y;
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

	Queue* queue = newQueue();
	Title** dist = malloc(n * sizeof(Title*));
	Title** distGen = malloc(n * sizeof(Title*));
	for (y = 0; y < n; y++)
	{
		dist[y] = malloc(m * sizeof(Title));
		distGen[y] = malloc(m * sizeof(Title));
	}

	Path startDrak, startGeneratorDrak, DrakGenerator;
	Path DrakPrincenza1GV, DrakPrincenza2GV, DrakPrincenza3GV;
	Path DrakPrincenza1GZ, DrakPrincenza2GZ, DrakPrincenza3GZ;
	Path GeneratorPrincenza1, GeneratorPrincenza2, GeneratorPrincenza3;
	Path P1P2GZ, P1P3GZ, P2P1GZ, P2P3GZ, P3P1GZ, P3P2GZ;
	Path P1P2GN, P1P3GN, P2P1GN, P2P3GN, P3P1GN, P3P2GN;
	Path P1G, P2G, P3G;

	P1G.cesta = P2G.cesta = P3G.cesta =
		DrakPrincenza1GV.cesta = DrakPrincenza2GV.cesta = DrakPrincenza3GV.cesta =
		DrakPrincenza1GZ.cesta = DrakPrincenza2GZ.cesta = DrakPrincenza3GZ.cesta =
		GeneratorPrincenza1.cesta = GeneratorPrincenza2.cesta = GeneratorPrincenza3.cesta =
		P1P2GZ.cesta = P1P3GZ.cesta = P2P1GZ.cesta = P2P3GZ.cesta = P3P1GZ.cesta = P3P2GZ.cesta =
		P1P2GN.cesta = P1P3GN.cesta = P2P1GN.cesta = P2P3GN.cesta = P3P1GN.cesta = P3P2GN.cesta =
		DrakGenerator.cesta = startGeneratorDrak.cesta = startDrak.cesta = NULL;
#ifdef _MSC_VER
#pragma endregion
#endif

	QV* start;

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
	}
	vytvorStartDrak(t, Drak, Generator, dist, distGen, &startDrak, &startGeneratorDrak);

	// PrincezneGeneratorOn
	if (Generator.x != -1)
	{
		vytvorCestu(Princezna1, distGen, &GeneratorPrincenza1);
		vytvorCestu(Princezna2, distGen, &GeneratorPrincenza2);
		vytvorCestu(Princezna3, distGen, &GeneratorPrincenza3);

		clear(distGen, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(distGen, Princezna1.x, Princezna1.y, ON);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, distGen, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		if (distGen[Princezna2.y][Princezna2.x].time == distGen[Princezna3.y][Princezna3.x].time)
		{
			vytvorCestu(Princezna3, distGen, &P1P3GZ);
			vytvorCestu(Princezna2, distGen, &P1P2GZ);
		}
		else if (distGen[Princezna2.y][Princezna2.x].time < distGen[Princezna3.y][Princezna3.x].time)
			vytvorCestu(Princezna2, distGen, &P1P2GZ);
		else
			vytvorCestu(Princezna3, distGen, &P1P3GZ);

		clear(distGen, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(distGen, Princezna2.x, Princezna2.y, ON);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, distGen, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		if (distGen[Princezna1.y][Princezna1.x].time == distGen[Princezna3.y][Princezna3.x].time)
		{
			vytvorCestu(Princezna1, distGen, &P2P1GZ);
			vytvorCestu(Princezna3, distGen, &P2P3GZ);
		}
		else if (distGen[Princezna1.y][Princezna1.x].time < distGen[Princezna3.y][Princezna3.x].time)
			vytvorCestu(Princezna1, distGen, &P2P1GZ);
		else
			vytvorCestu(Princezna3, distGen, &P2P3GZ);

		clear(distGen, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(distGen, Princezna3.x, Princezna3.y, ON);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, distGen, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		if (distGen[Princezna1.y][Princezna1.x].time == distGen[Princezna2.y][Princezna2.x].time)
		{
			vytvorCestu(Princezna1, distGen, &P3P1GZ);
			vytvorCestu(Princezna2, distGen, &P3P2GZ);
		}
		else if (distGen[Princezna1.y][Princezna1.x].time < distGen[Princezna2.y][Princezna2.x].time)
			vytvorCestu(Princezna1, distGen, &P3P1GZ);
		else
			vytvorCestu(Princezna2, distGen, &P3P2GZ);


		clear(dist, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(dist, Drak.x, Drak.y, ON);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		vytvorCestu(Princezna1, dist, &DrakPrincenza1GZ);
		vytvorCestu(Princezna2, dist, &DrakPrincenza2GZ);
		vytvorCestu(Princezna3, dist, &DrakPrincenza3GZ);
	}

	// Cesta k drakovi bez generatora
	if (startDrak.cesta)
	{
		clear(dist, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(dist, Drak.x, Drak.y, OFF);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);

		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &DrakGenerator);

		// Drak->Princezne bez generatora
		vytvorCestu(Princezna1, dist, &DrakPrincenza1GV);
		vytvorCestu(Princezna2, dist, &DrakPrincenza2GV);
		vytvorCestu(Princezna3, dist, &DrakPrincenza3GV);

		clear(dist, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(dist, Princezna1.x, Princezna1.y, OFF);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		vytvorCestu(Princezna2, dist, &P1P2GN);
		vytvorCestu(Princezna3, dist, &P1P3GN);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &P1G);

		clear(dist, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(dist, Princezna2.x, Princezna2.y, OFF);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		vytvorCestu(Princezna1, dist, &P2P1GN);
		vytvorCestu(Princezna3, dist, &P2P3GN);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &P2G);

		clear(dist, n, m, Drak, Princezna1, Princezna2, Princezna3, Generator);
		start = newStart(dist, Princezna3.x, Princezna3.y, OFF);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, INT_MAX, Drak, Princezna1, Princezna2, Princezna3, Generator);
		vytvorCestu(Princezna1, dist, &P3P1GN);
		vytvorCestu(Princezna2, dist, &P3P2GN);
		if (Generator.x != -1)
			vytvorCestu(Generator, dist, &P3G);
	}

	// Cesta k drakovi s generatorom
	if (startGeneratorDrak.cesta)
	{
		// Existuje cesta Start->Drak->Generator v case t?
		if (DrakGenerator.cesta)
		{
			if (startGeneratorDrak.time > DrakGenerator.time + startDrak.time)
			{
				free(startGeneratorDrak.cesta);
				startGeneratorDrak.cesta = NULL;
				//printf("# Aktivovat generator po drakovi je rychlejsie ako pred drakom\n");
			}
		}
	}

	PathList list;
	list.time = INT_MAX;
	list.parts = NULL;

	// Start->Generator->Drak->Princezna->Princezna->Princezna
	if (startGeneratorDrak.cesta)
	{
		updateList(&list, 4, &startGeneratorDrak, &DrakPrincenza1GZ, &P1P2GZ, &P2P3GZ);
		updateList(&list, 4, &startGeneratorDrak, &DrakPrincenza1GZ, &P1P3GZ, &P3P2GZ);

		updateList(&list, 4, &startGeneratorDrak, &DrakPrincenza2GZ, &P2P1GZ, &P1P3GZ);
		updateList(&list, 4, &startGeneratorDrak, &DrakPrincenza2GZ, &P2P3GZ, &P3P1GZ);

		updateList(&list, 4, &startGeneratorDrak, &DrakPrincenza3GZ, &P3P1GZ, &P1P2GZ);
		updateList(&list, 4, &startGeneratorDrak, &DrakPrincenza3GZ, &P3P2GZ, &P2P1GZ);
	}

	// Start->Drak->...
	if (startDrak.cesta)
	{
		// Start->Drak->Generator->Princezna->Princezna->Princezna
		if (DrakGenerator.cesta)
		{
			updateList(&list, 5, &startDrak, &DrakGenerator, &GeneratorPrincenza1, &P1P2GZ, &P2P3GZ);
			updateList(&list, 5, &startDrak, &DrakGenerator, &GeneratorPrincenza1, &P1P3GZ, &P3P1GZ);

			updateList(&list, 5, &startDrak, &DrakGenerator, &GeneratorPrincenza2, &P2P1GZ, &P1P3GZ);
			updateList(&list, 5, &startDrak, &DrakGenerator, &GeneratorPrincenza2, &P2P3GZ, &P3P1GZ);

			updateList(&list, 5, &startDrak, &DrakGenerator, &GeneratorPrincenza3, &P3P1GZ, &P1P2GZ);
			updateList(&list, 5, &startDrak, &DrakGenerator, &GeneratorPrincenza3, &P3P2GZ, &P2P1GZ);
		}

		// Start->Drak->Princezna->Princezna->Princezna
		updateList(&list, 4, &startDrak, &DrakPrincenza1GV, &P1P2GN, &P2P3GN);
		updateList(&list, 4, &startDrak, &DrakPrincenza1GV, &P1P3GN, &P3P2GN);

		updateList(&list, 4, &startDrak, &DrakPrincenza2GV, &P2P1GN, &P1P3GN);
		updateList(&list, 4, &startDrak, &DrakPrincenza2GV, &P2P3GN, &P3P1GN);

		updateList(&list, 4, &startDrak, &DrakPrincenza3GV, &P3P1GN, &P1P2GN);
		updateList(&list, 4, &startDrak, &DrakPrincenza3GV, &P3P2GN, &P2P1GN);

		if (Generator.x != -1)
		{
			// Start->Drak->Princezna->Generator->Princezna->Princezna
			updateList(&list, 5, &startDrak, &DrakPrincenza1GV, &P1G, &GeneratorPrincenza2, &P2P3GZ);
			updateList(&list, 5, &startDrak, &DrakPrincenza1GV, &P1G, &GeneratorPrincenza3, &P3P2GZ);

			updateList(&list, 5, &startDrak, &DrakPrincenza2GV, &P2G, &GeneratorPrincenza1, &P1P3GZ);
			updateList(&list, 5, &startDrak, &DrakPrincenza2GV, &P2G, &GeneratorPrincenza3, &P3P2GZ);

			updateList(&list, 5, &startDrak, &DrakPrincenza3GV, &P3G, &GeneratorPrincenza1, &P1P2GZ);
			updateList(&list, 5, &startDrak, &DrakPrincenza3GV, &P3G, &GeneratorPrincenza2, &P2P1GZ);

			// Start->Drak->Princezna->Princezna->Generator->Princezna
			updateList(&list, 5, &startDrak, &DrakPrincenza1GV, &P1P2GN, &P2G, &GeneratorPrincenza3);
			updateList(&list, 5, &startDrak, &DrakPrincenza1GV, &P1P3GN, &P3G, &GeneratorPrincenza2);

			updateList(&list, 5, &startDrak, &DrakPrincenza2GV, &P2P1GN, &P1G, &GeneratorPrincenza3);
			updateList(&list, 5, &startDrak, &DrakPrincenza2GV, &P2P3GN, &P3G, &GeneratorPrincenza1);

			updateList(&list, 5, &startDrak, &DrakPrincenza3GV, &P3P1GN, &P1G, &GeneratorPrincenza2);
			updateList(&list, 5, &startDrak, &DrakPrincenza3GV, &P3P2GN, &P2G, &GeneratorPrincenza1);
		}
	}

	/*
#ifdef _MSC_VER
#pragma region Vypis
#endif
	vypisCestu(startDrak, TOSTRING(startDrak));
	vypisCestu(DrakGenerator, TOSTRING(DrakGenerator));
	vypisCestu(startGeneratorDrak, TOSTRING(startGeneratorDrak));

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
#ifdef _MSC_VER
#pragma endregion
#endif
	*/

	int * result;
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

			for (y = 0; y < end; y++)
				result[offset + y] = part->part->cesta[y];

			part = part->next;
		}
	}
	else
	{
		result = NULL;
		*dlzka_cesty = 0;
	}

	free(teleporty);
	for (y = 0; y < n; y++)
	{
		free(dist[y]);
		free(distGen[y]);
	}
	free(dist);
	free(distGen);

	return result;
}

#ifdef _MSC_VER

void main()
{
	int n = 10;
	int m = 20;
	char** mapa = malloc(n * sizeof(char*));
	int i;
	for (i = 0; i < n; i++)
		mapa[i] = malloc(m * sizeof(char));

	strncpy(mapa[0], "....................", m);
	strncpy(mapa[1], ".....1N.D.0.........", m);
	strncpy(mapa[2], "......N........P....", m);
	strncpy(mapa[3], "..H.................", m);
	strncpy(mapa[4], "...............P....", m);
	strncpy(mapa[5], "..G.0...............", m);
	strncpy(mapa[6], "...............P....", m);
	strncpy(mapa[7], "....................", m);
	strncpy(mapa[8], "...............1....", m);
	strncpy(mapa[9], "....................", m);

	for(i = 10; i < n; i++)
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
