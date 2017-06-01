/** * @file Hw2KCore.cc * @author Rosun * @version 0.1 * @section LICENSE * 2017/04/11 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "GraphLite.h"
#define VERTEX_CLASS_NAME(name) KCore##name
int K=0;//k'value of KCore
//Message struct 
typedef struct VertexMulval {
    int  cur_degree=0;   //degree of node
    int  is_deleted=0;   //flag bit
} VertexMulval;

//class KCoreInputFormatter: public InputFormatter
class VERTEX_CLASS_NAME(InputFormatter): public InputFormatter {
public:
    int64_t getVertexNum() {
        unsigned long long n;
        sscanf(m_ptotal_vertex_line, "%lld", &n);// read one long long number ,and let n=it
        m_total_vertex= n;//the number of vertex
        return m_total_vertex;
    }
    int64_t getEdgeNum() {
        unsigned long long n;
        sscanf(m_ptotal_edge_line, "%lld", &n);// read one long long number ,and let n=it
        m_total_edge= n;//the number of edge
        return m_total_edge;
    }
    int getVertexValueSize() {
        m_n_value_size = sizeof(VertexMulval);
        return m_n_value_size;
    }
    int getEdgeValueSize() {
        m_e_value_size = sizeof(int);
        return m_e_value_size;
    }
    int getMessageValueSize() {
        m_m_value_size = sizeof(int);
        return m_m_value_size;
    }
    void loadGraph() {
        unsigned long long last_vertex;
        unsigned long long from;
        unsigned long long to;
        VertexMulval mulval;
        mulval.is_deleted=0;
        mulval.cur_degree=0;
        int weight = 0;

        int  value = 0;//initial value=0 degree=0
        int outdegree = 0;//outdegree of node
        const char *line= getEdgeLine(); // Get edge line, for user. Read from current file offset. 
        printf("Excute loadGraph() on KCoreInputfomatter class\n");
        printf("m_total_edge= %ld\n",m_total_edge);
        sscanf(line, "%lld %lld %d", &from, &to, &weight );     //from=source node, to=dest node
        addEdge(from, to, &weight);      //add one edge form->to weight=0
        last_vertex = from;
        ++outdegree;
        for (int64_t i = 1; i < m_total_edge; ++i) {
            line= getEdgeLine();// Get edge line, for user. Read from current file offset. 
            sscanf(line, "%lld %lld", &from, &to);
            if (last_vertex != from) {
                //addVertex(last_vertex, &value, outdegree);//addVertex and it's PageRank value,outdegree
                value=outdegree;
                mulval.cur_degree=outdegree;
             addVertex(last_vertex,&mulval,outdegree);//set value=outdegree
                printf("Vertex ID= %lld, degree= %d \n",last_vertex,value);
                last_vertex = from;
                outdegree = 1;
            } else {
                ++outdegree;
            }
            addEdge(from, to, &weight);
        }
          value=outdegree;
          value=outdegree;
          mulval.cur_degree=outdegree;
          addVertex(last_vertex,&mulval,outdegree);
          printf("Vertex ID= %lld, cur_degree= %d is_deleted=%d \n",last_vertex,mulval.cur_degree,mulval.is_deleted);
    }
};

class VERTEX_CLASS_NAME(OutputFormatter): public OutputFormatter {
public:
    void writeResult() {
        int64_t vid;
        VertexMulval  mulval;//degree
        char s[1024];

        for (ResultIterator r_iter; ! r_iter.done(); r_iter.next() ) {
            r_iter.getIdValue(vid, &mulval);
            int n = sprintf(s, "%lld: %d\n", (unsigned long long)vid, mulval.cur_degree);
            if(mulval.cur_degree>=K)         
                writeNextResLine(s, n);
        }
    }
};

// An aggregator that records a double value to compute sum
class VERTEX_CLASS_NAME(Aggregator): public Aggregator<int> {
public:
    void init() {
        m_global = 0;  //aggregator global value of AggrValue
        m_local = 0;   //aggregator local value of AggrValue
    }
    void* getGlobal() {
        return &m_global;
    }
    void setGlobal(const void* p) {
        m_global = * (int *)p;
    }
    void* getLocal() {
        return &m_local;
    }
    void merge(const void* p) {
        m_global += * (int  *)p;
    }
    void accumulate(const void* p) {
        m_local += * (int  *)p;
    }
};
/** *V E Message */
class VERTEX_CLASS_NAME(): public Vertex <VertexMulval , int , int> {
public:
    void compute(MessageIterator* pmsgs) {
        printf("Excute compute(), MessageIterrator *pmsgs, pmsgs.size= %d\n ",pmsgs->m_vector_size);
        printf(" vertex: ID=%ld current Vertex's value: %d \n",getVertexId(),getValue().cur_degree);
        printf("is_deleted= %d\n",getValue().is_deleted);
        VertexMulval mulval;
        if (getSuperstep() == 0) {  
            //initial some param 
            printf("cuurent VertexID= %ld ,cur_degree= %d is_deleted=%d\n",getVertexId(),getValue().cur_degree,getValue().is_deleted);
        } else {
            if (getSuperstep() >= 2 && (getValue().is_deleted==0 ) ) {
                    int  global_val = * (int *)getAggrGlobal(0);  //Get global value of aggregator index=0
                    printf("at compute() on PageRankVertex class, global_val==%d\n",global_val);
                    if (global_val ==0) {   //judge convergence
                            voteToHalt();
                                       // is_deleted=1; 
                            return;
                    }
                }  
            if(getValue().is_deleted==0){
                    int  sum = 0;
                    for ( ; ! pmsgs->done(); pmsgs->next() ) {
                            sum+=1;
                    }

                    mulval.cur_degree=getValue().cur_degree-sum;//
                    printf("getValue().cur_degree-sum= %d, sum=%d, MessageIterratorSize=%d\n",mulval.cur_degree,sum,pmsgs->m_vector_size);
                    int acc=fabs(getValue().cur_degree-mulval.cur_degree);
                    accumulateAggr(0,&acc);
                    mutableValue()->cur_degree=mulval.cur_degree;  //mutableValue() return one pointer type is VertexValue
                }else{
                        printf("state is active ,but is_deleted!=0\n");
                        voteToHalt();
                }                  
        }

            if(getValue().cur_degree<K  && (getValue().is_deleted==0) ){  //Vertex's degree <=K 
                sendMessageToAllNeighbors(2);
                printf("ID=%ld current Vertex's value < K: %d < %d ,send Message to all Neighbors\n",getVertexId(),getValue().cur_degree,K);
                voteToHalt();
                mulval.is_deleted=1;
                mutableValue()->is_deleted=mulval.is_deleted ;  //delete this node
            } 
            if(getValue().is_deleted==0)
                printf("exist vertex: ID=%ld current Vertex's value: %d \n",getVertexId(),getValue().cur_degree);
   }
};

class VERTEX_CLASS_NAME(Graph): public Graph {
public:
    VERTEX_CLASS_NAME(Aggregator)* aggregator;
public:
    // argv[0]: PageRankVertex.so
    // argv[1]: <input path>
    // argv[2]: <output path>
    void init(int argc, char* argv[]) {
        printf("Excute init() at class KCoreGraph\n");
        setNumHosts(5);
        setHost(0, "localhost", 1411);
        setHost(1, "localhost", 1421);
        setHost(2, "localhost", 1431);
        setHost(3, "localhost", 1441);
        setHost(4, "localhost", 1451);

        if (argc < 3) {  //the number of param
           printf ("Usage: %s <input path> <output path>\n", argv[0]);
           exit(1);
        }

        m_pin_path = argv[1];//input file path
        m_pout_path = argv[2];//output file path
        K= atoi(argv[3]);
        printf("Input path: %s\n",m_pin_path);
        printf("Output path: %s\n",m_pout_path);
        printf("K= %d\n",K);
        aggregator = new VERTEX_CLASS_NAME(Aggregator)[1];
        regNumAggr(1);//set m_aggregator_cnt=param, aggregator count
        regAggr(0, &aggregator[0]);
    }
    void term() {
        delete[] aggregator;
    }
};

/* STOP: do not change the code below. */
extern "C" Graph* create_graph() {
    Graph* pgraph = new VERTEX_CLASS_NAME(Graph);
    printf("Excute create_graph()\n");
    pgraph->m_pin_formatter = new VERTEX_CLASS_NAME(InputFormatter);
    pgraph->m_pout_formatter = new VERTEX_CLASS_NAME(OutputFormatter);
    pgraph->m_pver_base = new VERTEX_CLASS_NAME();
    return pgraph;
}

extern "C" void destroy_graph(Graph* pobject) {
    printf("Excute destroy_graph()\n");
    delete ( VERTEX_CLASS_NAME()* )(pobject->m_pver_base);
    delete ( VERTEX_CLASS_NAME(OutputFormatter)* )(pobject->m_pout_formatter);
    delete ( VERTEX_CLASS_NAME(InputFormatter)* )(pobject->m_pin_formatter);
    delete ( VERTEX_CLASS_NAME(Graph)* )pobject;
}