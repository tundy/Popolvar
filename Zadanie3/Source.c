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

typedef struct pathvalue
{
	Point point;
	struct pathvalue* back;
} PathValue;

typedef struct pathBack
{
	int steps;
	int time;
	PathValue* start;
} Path;

typedef struct qv
{
	int slowed;
	Point point;
	Point back;
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
	Point back;
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
			dist[y][x].steps = 0;
			distGen[y][x].steps = 0;
		}
	}
}

QV* newQV(QV* value, int y, int x)
{
	QV* new_qv = malloc(sizeof(QV));
	new_qv->back.y = value->point.y;
	new_qv->back.x = value->point.x;
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
	dist[value->point.y][value->point.x].steps = dist[value->back.y][value->back.x].steps + 1;
}

QV* newStart(Title** dist, Title** distGen, int x, int y, int gen)
{
	QV* value = malloc(sizeof(QV));
	value->point.x = x;
	value->point.y = y;
	value->back.x = x;
	value->back.y = y;
	value->generator = gen;
	value->slowed = 0;
	if (gen)
		distGen[x][y].time = 0;
	else
		dist[x][y].time = 0;
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
			time += distGen[value->back.y][value->back.x].time;
			if(time < 0 || time > maxTime)
				continue;
			if (time < distGen[value->point.y][value->point.x].time)
			{
				updateDist(distGen, value, time);
				addToQueue(mapa, n, m, teleporty, queue, value);
			}
		}
		else
		{
			time += dist[value->back.y][value->back.x].time;
			if (time < 0 || time > maxTime)
				continue;
			if (time < dist[value->point.y][value->point.x].time)
			{
				updateDist(dist, value, time);
				addToQueue(mapa, n, m, teleporty, queue, value);
				if (mapa[value->back.y][value->back.x] == gene)
				{
					value->generator = ON;
					updateDist(distGen, value, 1);
					addToQueue(mapa, n, m, teleporty, queue, value);
				}
			}
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
	QV * start = newStart(dist, distGen, startX, startY, mapa[startY][startX] == gene);
	UDLR(n, m, queue, start, start->point);
	dijkstra(mapa, n, m, teleporty, queue, dist, distGen, t);

	if (dist[Drak.y][Drak.x].steps > 0 && dist[Drak.y][Drak.x].steps <= t)
	{
		printf("Start->Drak bez generatora v case %d po %d polickach\n", dist[Drak.y][Drak.x].time, dist[Drak.y][Drak.x].steps);

	}

	if (distGen[Drak.y][Drak.x].steps > 0 && distGen[Drak.y][Drak.x].steps <= t)
	{
		if(dist[Generator.y][Generator.x].steps)
		{
			int realSteps = dist[Generator.y][Generator.x].steps + distGen[Drak.y][Drak.x].steps;
			if (realSteps <= t)
			{
				printf("Start->Drak s generatorom v case %d po %d polickach\n", dist[Generator.y][Generator.x].time + distGen[Drak.y][Drak.x].time, realSteps);
			}
		}
		else
		{
			printf("Start->Drak s generatorom v case %d po %d polickach\n", distGen[Drak.y][Drak.x].time, distGen[Drak.y][Drak.x].steps);
		}
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
	int* cesta = zachran_princezne(mapa, n, m, 15, &dlzka_cesty);

	printf("%d\n", dlzka_cesty);
	for (i = 0; i < dlzka_cesty; ++i)
		printf("%d %d\n", cesta[i * 2], cesta[i * 2 + 1]);

	for (i = 0; i < n; i++)
		free(mapa[i]);
	free(mapa);

	getchar();
}

#endif
