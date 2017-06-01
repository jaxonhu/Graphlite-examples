/* Group 3, 201628015029059, hujiaxuan */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include "GraphLite.h"

#define VERTEX_CLASS_NAME(name) TriangleCount##name



using namespace std;


typedef struct VertexC {

	int64_t vid;

	int in = 0;
	int out = 0;
	int through = 0;
	int circle = 0;

	vector<int64_t> in_list;  //in neighbors
	vector<int64_t> out_list; // out neighbors

} VertexC;



class VERTEX_CLASS_NAME(InputFormatter): public InputFormatter {
public:
    int64_t getVertexNum() {
        unsigned long long n;
        sscanf(m_ptotal_vertex_line, "%lld", &n);
        m_total_vertex= n;
        return m_total_vertex;
    }
    int64_t getEdgeNum() {
        unsigned long long n;
        sscanf(m_ptotal_edge_line, "%lld", &n);
        m_total_edge= n;
        return m_total_edge;
    }
    int getVertexValueSize() {
        m_n_value_size = sizeof(VertexC);
        return m_n_value_size;
    }
    int getEdgeValueSize() {
        m_e_value_size = sizeof(int);
        return m_e_value_size;
    }
    int getMessageValueSize() {
        m_m_value_size = sizeof(int64_t); 
       return m_m_value_size;
    }
    void loadGraph() {
        unsigned long long last_vertex;
        unsigned long long from;
        unsigned long long to;
        int weight = 0;

        
        int outdegree = 0;
        
        const char *line= getEdgeLine();

        sscanf(line, "%lld %lld", &from, &to);
        addEdge(from, to, &weight);

        last_vertex = from;
        ++outdegree;

        VertexC value;

        value.out_list.push_back(to);
        for (int64_t i = 1; i < m_total_edge; ++i) {
            line= getEdgeLine();
            sscanf(line, "%lld %lld", &from, &to);  
            if (last_vertex != from) {
            	value.vid = last_vertex;
                addVertex(last_vertex, &value, outdegree);
                last_vertex = from;
                outdegree = 1;
                memset(&value,0,sizeof(VertexC));  
                value.out_list.push_back(to);
            } else {
                value.out_list.push_back(to);
                ++outdegree;
            }
            addEdge(from, to, &weight);
        }
        value.vid = last_vertex;
        addVertex(last_vertex, &value, outdegree);
    }
};

class VERTEX_CLASS_NAME(OutputFormatter): public OutputFormatter {
public:
    void writeResult() {
       //TODO
    	int64_t vid;
    	char s[1024];
    	VertexC value;
    
    	ResultIterator r_iter;
    	r_iter.next();
    	r_iter.getIdValue(vid,&value);
    	printf("before writeResult\n");
    	int n = sprintf(s,"in:%d\nout:%d\nthrough:%d\ncircle:%d\n",value.in,value.out,value.through,value.circle);
    	writeNextResLine(s, n);
    	printf("writeResult\n");
    }
};

// An aggregator that records a double value tom compute sum
class VERTEX_CLASS_NAME(Aggregator): public Aggregator<int> {
public:
    void init() {
        m_global = 0;
        m_local = 0;
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
        m_global += * (int *)p;
    }
    void accumulate(const void* p) {
        m_local += * (int *)p;
    }
};

class VERTEX_CLASS_NAME(): public Vertex <VertexC, int, int64_t> {
public:
    void compute(MessageIterator* pmsgs) {

        int in_triangle = 0;
        int out_triangle = 0;
        int through_triangle = 0;
        int circle_triangle = 0;

        
    	VertexC vertexc;
		vertexc.vid = getValue().vid;    	

		printf("Excute compute(), MessageIterrator *pmsgs, pmsgs.size= %d\n ",pmsgs->m_vector_size);
        printf(" vertex: ID=%ld current In_list_size: %ld Out_list_size: %ld \n",getVertexId(),getValue().in_list.size(),getValue().out_list.size());
       
        if (getSuperstep() == 0) {
        	int64_t vertex_id = getVertexId();
        	sendMessageToAllNeighbors(vertex_id);
        } else {
            if (getSuperstep() == 2) {
                
                printf("when getSuperstep() == 2, MessageIterrator *pmsgs, pmsgs.size= %d\n ",pmsgs->m_vector_size);
                for (; ! pmsgs->done(); pmsgs->next()){
                	
                	int64_t msg_vid = pmsgs->getValue();
                	
                    for(int i = 0;i < getValue().in_list.size(); i++){
                    	if(getValue().in_list[i] == msg_vid){
                    		through_triangle++;
                    	}
                    }
                    for(int j = 0; j < getValue().out_list.size(); j++){
                    	if(getValue().out_list[j] == msg_vid){
                    		circle_triangle++;
                    	}
                    }
                    
                }

                printf("when getSuperstep() >= 2, vertexId(%ld) through_triangle: %d ,circle_triangle: %d\n",getVertexId(),through_triangle,circle_triangle);

                VertexC vertexc;
                vertexc.vid = getVertexId();
                vertexc.in = through_triangle;
                vertexc.out = through_triangle;
                vertexc.through = through_triangle;
                vertexc.circle = circle_triangle;

                accumulateAggr(0,&through_triangle);
                accumulateAggr(1,&circle_triangle);
                * mutableValue() = vertexc;
                sendMessageToAllNeighbors(getVertexId());        
                voteToHalt();
            }
            if(getSuperstep() == 3){

            	VertexC vertexc;
            	vertexc.vid = getVertexId();
            	int through = *(int *)getAggrGlobal(0);
            	int circle = *(int *)getAggrGlobal(1);
            	vertexc.in = through;
            	vertexc.out = through;
            	vertexc.through = through;
            	vertexc.circle = circle/3;
            
            	* mutableValue() = vertexc;
            	printf("when getSuperstep == 4  vertexId(%ld) through=%ld circle=%ld\n ",getVertexId(),through,circle);
            	voteToHalt();
            	return;
            }
            if(getSuperstep() == 1){
            	for ( ; ! pmsgs->done(); pmsgs->next() ){
               	 	int64_t neighbor_id = pmsgs->getValue();
                	printf("when getSuperstep() == 1, vertexId= %ld, neighbor_vid= %lld \n",getVertexId(),neighbor_id);
                	vertexc.in_list.push_back(neighbor_id);
            	}
            
           		for(int i = 0;i < getValue().out_list.size();i++){
            		vertexc.out_list.push_back(getValue().out_list[i]);
          		}

            	vertexc.vid = getVertexId();
            	*mutableValue() = vertexc;
     
            	for(int i = 0; i < vertexc.in_list.size(); i++){
            		int64_t msg_vid = vertexc.in_list[i];
            		printf("vertexId %ld already sendMessageToAll   msgID %ld\n",getVertexId(),msg_vid);
            		sendMessageToAllNeighbors(msg_vid);
            	}
           	    voteToHalt();
            }
            
        }
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

        setNumHosts(5);
        setHost(0, "localhost", 1411);
        setHost(1, "localhost", 1421);
        setHost(2, "localhost", 1431);
        setHost(3, "localhost", 1441);
        setHost(4, "localhost", 1451);

        if (argc < 3) {
           printf ("Usage: %s <input path> <output path>\n", argv[0]);
           exit(1);
        }

        m_pin_path = argv[1];
        m_pout_path = argv[2];

        aggregator = new VERTEX_CLASS_NAME(Aggregator)[2];
        regNumAggr(2);
        regAggr(0, &aggregator[0]);
        regAggr(1, &aggregator[1]);
    }

    void term() {
        delete[] aggregator;
    }
};

/* STOP: do not change the code below. */
extern "C" Graph* create_graph() {
    Graph* pgraph = new VERTEX_CLASS_NAME(Graph);

    pgraph->m_pin_formatter = new VERTEX_CLASS_NAME(InputFormatter);
    pgraph->m_pout_formatter = new VERTEX_CLASS_NAME(OutputFormatter);
    pgraph->m_pver_base = new VERTEX_CLASS_NAME();

    return pgraph;
}

extern "C" void destroy_graph(Graph* pobject) {
    delete ( VERTEX_CLASS_NAME()* )(pobject->m_pver_base);
    delete ( VERTEX_CLASS_NAME(OutputFormatter)* )(pobject->m_pout_formatter);
    delete ( VERTEX_CLASS_NAME(InputFormatter)* )(pobject->m_pin_formatter);
    delete ( VERTEX_CLASS_NAME(Graph)* )pobject;
}


