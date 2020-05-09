typedef struct 
{
	int from;
	int to;
} Request;

typedef struct
{
	Request *reQueue;
    int count;
} Buffer;

int main(int, char**);

void* request( void* );

void* processReq( void* );
