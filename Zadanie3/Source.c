#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

/*void clear(Queue* queue)
{
while (queue->first)
{
qValue* temp = queue->first;
queue->first = queue->first->next;
free(temp);
}
}*/

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

void clear(Title** dist, Title** distGen, int n, int m)
{
	int x, y;
	for (y = 0; y < n; y++)
	{
		for (x = 0; x < m; x++)
		{
			dist[y][x].time = INT_MAX;
			distGen[y][x].time = INT_MAX;
			dist[y][x].back = NULL;
			distGen[y][x].back = NULL;
			dist[y][x].steps = -1;
			distGen[y][x].steps = -1;
		}
	}
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

void updateDist(Title** dist, QV* value, int time)
{
	dist[value->point.y][value->point.x].time = time;
	dist[value->point.y][value->point.x].steps = dist[value->back->y][value->back->x].steps + 1;
	dist[value->point.y][value->point.x].back = value->back;
}

QV* newStart(Title** dist, Title** distGen, int x, int y, int gen)
{
	QV* value = malloc(sizeof(QV));
	value->point.x = x;
	value->point.y = y;
	value->back = &value->point;
	value->generator = gen;
	value->slowed = 0;
	if (gen)
	{
		distGen[y][x].time = 0;
		distGen[y][x].steps = 0;
	}
	else
	{
		dist[y][x].time = 0;
		dist[y][x].steps = 0;
	}
	return value;
}

void addToQueue(char** mapa, int n, int m, Teleport** teleporty, Queue* queue, QV* value)
{
	if (value->generator && isTeleport(mapa[value->point.y][value->point.x]))
	{
		Teleport* teleport = teleporty[mapa[value->point.y][value->point.x] - '0'];
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

void dijkstra(char** mapa, int n, int m, Teleport** teleporty, Queue* queue, Title** dist, Title** distGen, int maxTime)
{
	while (any(queue))
	{
		QV* value = top(queue);
		pop(queue);

		if (mapa[value->point.y][value->point.x] == wall)
			continue;

		if (!value->slowed++ && mapa[value->point.y][value->point.x] == slow)
		{
			enqueue(queue, value);
			continue;
		}

		int time = mapa[value->point.y][value->point.x] == slow ? 2 : 1;

		if (value->generator)
		{
			time += distGen[value->back->y][value->back->x].time;
			if (time < 0 || time > maxTime)
				continue;
			if (time < distGen[value->point.y][value->point.x].time)
			{
				updateDist(distGen, value, time);
				addToQueue(mapa, n, m, teleporty, queue, value);
			}
		}
		else
		{
			time += dist[value->back->y][value->back->x].time;
			if (time < 0 || time > maxTime)
				continue;
			if (time < dist[value->point.y][value->point.x].time)
			{
				updateDist(dist, value, time);
				addToQueue(mapa, n, m, teleporty, queue, value);
				if (mapa[value->point.y][value->point.x] == gene)
				{
					value->generator = ON;
					updateDist(distGen, value, 0);
					addToQueue(mapa, n, m, teleporty, queue, value);
				}
			}
		}
	}
}

void StartDrak(int t, Point Drak, Point Generator, Title** dist, Title** distGen, Path* startDrak, Path* startGenerator, Path* generatorDrak)
{
	if (dist[Generator.y][Generator.x].steps >= 0)
	{
		startGenerator->steps = dist[Generator.y][Generator.x].steps;
		startGenerator->time = dist[Generator.y][Generator.x].time;
		startGenerator->cesta = malloc(startGenerator->steps * 2 * sizeof(int));
		int index = dist[Generator.y][Generator.x].steps * 2;
		Point* p = &Generator;
		while (index > 0)
		{
			startGenerator->cesta[--index] = p->x;
			startGenerator->cesta[--index] = p->y;
			p = dist[p->y][p->x].back;
		}
	}
	if (distGen[Drak.y][Drak.x].steps >= 0)
	{
		generatorDrak->steps = distGen[Drak.y][Drak.x].steps;
		generatorDrak->time = distGen[Drak.y][Drak.x].time;
		generatorDrak->cesta = malloc(generatorDrak->steps * 2 * sizeof(int));
		int index = distGen[Drak.y][Drak.x].steps * 2;
		Point* p = &Drak;
		while (index > 0)
		{
			generatorDrak->cesta[--index] = p->x;
			generatorDrak->cesta[--index] = p->y;
			p = distGen[p->y][p->x].back;
		}
	}

	int startGenDrak = 0;
	if (generatorDrak->cesta != NULL)
		startGenDrak += generatorDrak->time;
	if (startGenerator->cesta != NULL)
		startGenDrak += startGenerator->time;

	// Ak sa k Drakovi pomocou generatora nedostanes ani v case t nema zmysel si to pamatat
	if(startGenDrak > t)
	{
		free(startGenerator->cesta);
		free(generatorDrak->cesta);
		startGenerator->cesta = NULL;
		generatorDrak->cesta = NULL;
	}

	// Ak cesta cez generator bola rychlejsie nema zmysel si pamatat cestu bez generatora
	if (dist[Drak.y][Drak.x].steps >= 0 && (generatorDrak->cesta == NULL || dist[Drak.y][Drak.x].steps < startGenDrak) )
	{
		startDrak->steps = dist[Drak.y][Drak.x].steps;
		startDrak->time = dist[Drak.y][Drak.x].time;
		startDrak->cesta = malloc(startDrak->steps * 2 * sizeof(int));
		int index = dist[Drak.y][Drak.x].steps * 2;
		Point* p = &Drak;
		while (index > 0)
		{
			startDrak->cesta[--index] = p->x;
			startDrak->cesta[--index] = p->y;
			p = dist[p->y][p->x].back;
		}
	}
}

int* zachran_princezne(char** mapa, int n, int m, int t, int* dlzka_cesty)
{
	int x, y;
	Point Drak, Princezna1, Princezna2, Princezna3, Generator;
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

	//for(y = 0; y < n; y++)
	//	printf("%.*s\n", m, mapa[y]);

	Queue* queue = newQueue();
	Title** dist = malloc(n * sizeof(Title*));
	Title** distGen = malloc(n * sizeof(Title*));
	for (y = 0; y < n; y++)
	{
		dist[y] = malloc(m * sizeof(Title));
		distGen[y] = malloc(m * sizeof(Title));
	}

	clear(dist, distGen, n, m);
	QV* start = newStart(dist, distGen, startX, startY, mapa[startY][startX] == gene);
	UDLR(n, m, queue, start, start->point);
	dijkstra(mapa, n, m, teleporty, queue, dist, distGen, t);

	Path startDrak, startGenerator, generatorDrak, DrakGenerator;
	startDrak.cesta = NULL;
	startGenerator.cesta = NULL;
	generatorDrak.cesta = NULL;
	DrakGenerator.cesta = NULL;

	StartDrak(t, Drak, Generator, dist, distGen, &startDrak, &startGenerator, &generatorDrak);

	if (startDrak.cesta != NULL)
	{
		clear(dist, distGen, n, m);
		start = newStart(dist, distGen, Drak.x, Drak.y, OFF);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, distGen, INT_MAX);

		if (dist[Generator.y][Generator.x].steps >= 0)
		{
			DrakGenerator.steps = dist[Generator.y][Generator.x].steps;
			DrakGenerator.time = dist[Generator.y][Generator.x].time;
			DrakGenerator.cesta = malloc(DrakGenerator.steps * 2 * sizeof(int));
			int index = dist[Generator.y][Generator.x].steps * 2;
			Point* p = &Generator;
			while (index > 0)
			{
				DrakGenerator.cesta[--index] = p->x;
				DrakGenerator.cesta[--index] = p->y;
				p = dist[p->y][p->x].back;
			}
		}

		// DEBUG:
		printf("Start->Drak bez generatora v case %d po %d polickach\n", startDrak.time, startDrak.steps);
		int i;
		for (i = 0; i < startDrak.steps; ++i)
			printf("[%d;%d] ", startDrak.cesta[i * 2], startDrak.cesta[i * 2 + 1]);
		putchar('\n');
	}
	else
	{
		clear(dist, distGen, n, m);
		start = newStart(dist, distGen, Drak.x, Drak.y, ON);
		UDLR(n, m, queue, start, start->point);
		dijkstra(mapa, n, m, teleporty, queue, dist, distGen, INT_MAX);
	}
	if (startGenerator.cesta != NULL)
	{
		printf("Start->Generator v case %d po %d polickach\n", startGenerator.time, startGenerator.steps);
		int i;
		for (i = 0; i < startGenerator.steps; ++i)
			printf("[%d;%d] ", startGenerator.cesta[i * 2], startGenerator.cesta[i * 2 + 1]);
		putchar('\n');
	}
	if (generatorDrak.cesta != NULL)
	{
		printf("Generator->Drak v case %d po %d polickach\n", generatorDrak.time, generatorDrak.steps);
		int i;
		for (i = 0; i < generatorDrak.steps; ++i)
			printf("[%d;%d] ", generatorDrak.cesta[i * 2], generatorDrak.cesta[i * 2 + 1]);
		putchar('\n');
	}
	if (DrakGenerator.cesta != NULL)
	{
		printf("Drak->Generator v case %d po %d polickach\n", DrakGenerator.time, DrakGenerator.steps);
		int i;
		for (i = 0; i < DrakGenerator.steps; ++i)
			printf("[%d;%d] ", DrakGenerator.cesta[i * 2], DrakGenerator.cesta[i * 2 + 1]);
		putchar('\n');
	}


	int* result = NULL;
	*dlzka_cesty = 0;

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
	{
		mapa[i] = malloc(m * sizeof(char));
	}

	strncpy(mapa[0], "....................", m);
	strncpy(mapa[1], ".....1N.D.0.........", m);
	strncpy(mapa[2], "H.....N........P....", m);
	strncpy(mapa[3], "....................", m);
	strncpy(mapa[4], "..H............P....", m);
	strncpy(mapa[5], "..G.0...............", m);
	strncpy(mapa[6], "...............P....", m);
	strncpy(mapa[7], "....................", m);
	strncpy(mapa[8], "...............1....", m);
	strncpy(mapa[9], "....................", m);

	int dlzka_cesty;
	int* cesta = zachran_princezne(mapa, n, m, 100, &dlzka_cesty);

	printf("%d\n", dlzka_cesty);
	for (i = 0; i < dlzka_cesty; ++i)
		printf("%d %d\n", cesta[i * 2], cesta[i * 2 + 1]);

	for (i = 0; i < n; i++)
		free(mapa[i]);
	free(mapa);

	getchar();
}

#endif
