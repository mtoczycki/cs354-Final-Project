#define PIPE_RW 2 /* Indicates pipe can be read and written to */
#define PIPE_R 1 /* Indicates pipe can be read from */
#define PIPE_FREE 0 /* Indicates the pipe table entry is empty */
#define NPIPES 10
#define PIPE_LEN 32
#define EOF_INDICATOR 0xff00

struct pipe{
	int32 pstatus;/* The status of the pipe */
	int16 buff[PIPE_LEN];/* The statically allocated circular buffer of the pipe */
	int rindex;
	int windex;
	sid32 Producer;
	sid32 Consumer;
	sid32 Mutex;
};
extern struct pipe pipes[];
