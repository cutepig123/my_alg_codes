#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <locale>
#include <map>
#include <assert.h>
#include <math.h>

using namespace std;

#define DBG(x, format) printf(#x "=" format "\n", (x))
//#define DBG(x, format) 

namespace Baidu09_3
{
	struct   Point   {                       
		double   x,   y;   
	};

	struct Node
	{
		Point ptLT, ptRB;
		bool isLink;
		string name;
		
		vector <Node> child;	
	};

	Node * SrchNode(Node *root, string name)
	{
		Node * p = 0;

		if(root->name == name)
			return root;

		for(size_t i=0; i<root->child.size(); i++)
		{
			p= SrchNode(root->child[i]);
			if(p)
				return p;
		}

		return 0;
	}

	bool isCharNum(char c)
	{
		return isalnum(c);
	}

	char line[10000];

	int main()
	{
		int m,n,i;
		scanf("%d%d\n", &m, &n);
		DBG(m,"%d");
		DBG(n,"%d");
		
		//Node * tree = 0;
	
		for(i=0; i<m; i++)
		{
			char token[100];
			fgets(line, 9999, stdin);
			
			DBG(line, "%s");
			//bool start =1;
			for(char *p = line;  *p; )
			{
				//Node *node;
				
				int j=0;
				for(; isCharNum(*p); j++, p++)
				 token[j] = *p;
				if(j==0)break;

				token[j] = 0;

				/*if(mTree.find(token) == mTree.end())
					node = mTree[token] = new Node;
				else
					node = mTree[token];*/

				if(*p=='(')
				{
					while(*p!=')') p++;
					//node->isLink = true;
				}
				else
					//node->isLink = false;
			
				while(*p && !isCharNum(*p))	p++;

				DBG(token, "%s");
				//tree = (node);
			}
		}
		return 0;
	}
};
