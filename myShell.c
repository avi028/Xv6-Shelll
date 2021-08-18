#include "types.h"
#include "user.h"
#include "fcntl.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define COND  5
#define NILL  6
#define MAXARGS 10
#define AND 88
#define OR 11

struct token{
    char *list[100];
    int len;
};

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *ifile;
  char *ofile;
  int imode;
  int omode;
  int ifd;
  int ofd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct condcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
  int condition;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct branchcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

/*Constructors*/

struct token *
token(void)
{
    struct token * t;
    t= (struct token *)malloc(sizeof(struct token));
    memset(t,0,sizeof(struct token));
    return (struct token *)t;
}


struct cmd*
execcmd(void)
{
  struct execcmd *cmd;
  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *ifile,char *ofile)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->ifile = ifile;
  cmd->ofile = ofile;
  cmd->ifd = 0;
  cmd->ofd = 1;
  cmd->imode = O_RDONLY;
  cmd->omode =  O_WRONLY|O_CREATE;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
condcmd(struct cmd *left, struct cmd *right,int cond)
{
  struct condcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = COND;
  cmd->left = left;
  cmd->right = right;
  cmd->condition = cond;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

/*Functions Declerations*/

struct token * parseBuf(char * buf,int buflen);
struct cmd * getLastCmd(struct cmd * c);
struct cmd * parsetokenlist(struct token* t);
int getcmd(char *buf, int nbuf);
char* iswhiteSpace(char s);
char* isSymbol(char s);
int isAlphaNum(char s) ;
void runcmd(struct cmd *);
int strcpy_shell(char *s,char *t);
void panic(char *s);


/*Main Code */
int
main(void)
{
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
 static char buf[100];
  while(getcmd(buf, sizeof(buf)) >= 0)
  {
    if(buf[0]=='\n')
        continue;  
    else if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ')
    {
      buf[strlen(buf)-1] = 0;
      if(chdir(buf+3) < 0)
        printf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    else if(buf[0]=='e' && buf[1]=='x' && buf[2]=='i' && buf[3]=='t')
    {
        while(wait(0)!=-1)
        {
            continue;
        }
        exit(0);
    }
    int pid=0,status=-1;
    if((pid=fork()) == 0)
    {
       runcmd(parsetokenlist(parseBuf(&buf[0],strlen(buf))));
    }
    printf(1,"pid %d forked\n",pid);
    if(pid == wait(&status)){printf(1,"pid %d exit %d\n",pid,status);}
  }
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  printf(2, "myshell> ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

char * 
iswhiteSpace(char s)
{
    return strchr(whitespace,s);
}

char * 
isSymbol(char s)
{
    return strchr(symbols, s);
}

int
isAlphaNum(char s)
{
    if((s>=65 && s<=90 )|| (s>=97 && s<=122) || (s>=46 && s<=57))
    {
        return 1;
    }
    return 0;
}

int
strcpy_shell(char *s,char *t)
{
    int count=0;
    while((*s++=*t++)!=0)
        count++;
    return count;
}

void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit(-1);
}


int 
tokenize(char ** ptrtokenlist,char ** ptrbuf,int buflen)
{
    int i = 0;
    char * buf = *ptrbuf;    
    int tokenlistitr=0;

    for(i=0;i<buflen;i++)
    {
        if(iswhiteSpace(buf[i]))
        {
            buf[i]=0;
        }
        else if(isSymbol(buf[i]))
        {
            ptrtokenlist[tokenlistitr++]=&buf[i];
            while(isSymbol(buf[i]) && i<buflen)
                i++;
            i--;
        }
        else if(isAlphaNum(buf[i]))
        {
            ptrtokenlist[tokenlistitr++]=&buf[i];            
            while(isAlphaNum(buf[i]) && i<buflen)
                i++;
            i--;
        }
        else
        {
            return -1;
        }
    }   
    return tokenlistitr;
}



struct token *
parseBuf(char * buf,int buflen)
{
    struct token * t = token();
    t->len = tokenize(&(t->list[0]),&buf,buflen);    
    if(t->len==-1)
    {
        printf(1,"ERROR : Illegal Character\n");
        free(t);
        exit(-1);
    }
    else if(t->len==0)
    {
        exit(0);
    }
    return (struct token *)t;
}

struct cmd *
getLastCmd(struct cmd * c)
{
    struct branchcmd * b;
    switch(c->type)
    {
        case LIST:
        case PIPE:
        case COND:
        {
            b = (struct branchcmd *)c;
            if(b->right==0 || b->right->type==EXEC || b->right->type==REDIR) 
                return c;
            else
                return getLastCmd((struct cmd *)b->right);   
            break;
        }
        default:
        {
            return c;
            printf(1,"ERROR in getLastCmd\n");
            break;
        }
    }
    return 0;
}

/*
cprintf
 ls ; cat < README || cat > README2 | grep xv6 > helloworld.txt
char symbols[] = "<|>&;()";

*/
struct cmd *
parsetokenlist(struct token* t)
{
    int itr=0;
    struct cmd *fcmd=0;
    for(itr=0;itr<t->len;itr++)
    {
        printf(1,"itr %d: %s\n",itr,t->list[itr]);
    }
    for(itr=0;itr<t->len;itr++)
    {
//        printf(1,"%c\n",t->list[itr][0]);
        switch(t->list[itr][0]){
            case '|': 
                    {
                        if(fcmd==0)
                        {printf(1,"Illegal command\n");exit(-1);}

                        if(strlen(t->list[itr])==1)
                        {
                            struct cmd * c = getLastCmd(fcmd);
                            switch(c->type)
                            {
                                case COND:
                                case LIST:
                                case PIPE:
                                {
                                    struct branchcmd *b = (struct branchcmd *)c;
                                    if(b->right==0)
                                    {
                                        printf(1,"Illegal Syntax\n");
                                        exit(-1);
                                    }
                                    else
                                    {
                                        if(b->right->type==REDIR)
                                        {
                                            struct redircmd * r = (struct redircmd *)b->right;

                                            if(r->ofile!=0)
                                            {
                                                printf(1," No Input to pipe ouput redirected to file ERROR \n");
                                                exit(-1);
                                            }
                                            else
                                            {
                                                b->right = pipecmd(b->right,0);                                        
                                                break;
                                            }

                                        }
                                        else
                                        {
                                            b->right = pipecmd(b->right,0);                                        
                                        }
                                        break;
                                    }
                                }
                                case EXEC:
                                {
                                    fcmd = pipecmd(c,0);
                                    break;
                                }
                                case REDIR:
                                {
                                    struct redircmd * r = (struct redircmd *)c;

                                    if(r->ofile!=0)
                                    {
                                        printf(1," No Input to pipe ouput redirected to file ERROR \n");
                                        exit(-1);
                                    }
                                    else
                                    {
                                        fcmd = pipecmd(c,0);
                                        break;
                                    }
                                }
                                default:
                                    printf(1,"Error in Token Parser\n");
                                    break;
                            }

                        }   
                        else if((strlen(t->list[itr])==2) && (t->list[itr][1]=='|'))
                        {
                            if(fcmd==0)
                            {printf(1,"Illegal command\n");exit(-1);}
                            struct cmd * c = getLastCmd(fcmd);
                            switch(c->type)
                            {
                                case COND:
                                case LIST:
                                case PIPE:
                                {
                                    struct branchcmd *b = (struct branchcmd *)c;
                                    if(b->right==0)
                                    {
                                        printf(1,"Illegal Syntax\n");
                                        exit(-1);
                                    }
                                    else
                                    {
                                        fcmd = condcmd(fcmd,0,OR);
                                    }
                                    break;
                                }
                                case EXEC:
                                case REDIR:
                                {
                                    fcmd = condcmd(fcmd,0,OR);
                                    break;
                                }
                                default:
                                    printf(1,"Error in Token Parser\n");
                                    break;
                            }
                        }
                        else
                        {
                            printf(1,"Error !! ");
                            exit(-1);
                        }
                        break;
                    }
            case '&':
                    {
                        if((strlen(t->list[itr])==2) && (t->list[itr][1]=='&'))
                        {
                            if(fcmd==0)
                            {printf(1,"Illegal command\n");exit(-1);}
                            struct cmd * c = getLastCmd(fcmd);
                            switch(c->type)
                            {
                                case COND:
                                case LIST:
                                case PIPE:
                                {
                                    struct branchcmd *b = (struct branchcmd *)c;
                                    if(b->right==0)
                                    {
                                        printf(1,"Illegal Syntax\n");
                                        exit(-1);
                                    }
                                    else
                                    {
                                        fcmd = condcmd(fcmd,0,AND);
                                    }
                                    break;
                                }
                                case EXEC:
                                case REDIR:
                                {
                                    fcmd = condcmd(fcmd,0,AND);
                                    break;
                                }
                                default:
                                    printf(1,"Error in Token Parser\n");
                                    break;
                            }
                        }   
                        else
                        {
                            printf(1,"Illegal command\n");
                            exit(-1);
                        }
                        break;
                    }
            case ';':
                     {
                        if(fcmd==0)
                        {
                            printf(1,"Illegal command\n");
                            exit(0);
                        }
                        if(strlen(t->list[itr])==1)
                        {
                            struct cmd * e =  0;
                            fcmd = listcmd((struct cmd *)fcmd, (struct cmd *)e);   
                        }   
                        else
                        {
                            printf(1,"Illegal Commands | <List type> ");
                            exit(-1);
                        }
                        break;
                    }
            case '>':/*Read the file from next token*/
                    {
                        if(fcmd==0)
                        {printf(1,"Illegal command\n");exit(0);}

                        if(strlen(t->list[itr])==1)
                        {
                            struct redircmd * r;
                            struct cmd * c = getLastCmd(fcmd);

                            switch(c->type)
                            {
                                case LIST:
                                case PIPE:
                                case COND:
                                {
                                    struct branchcmd *b = (struct branchcmd *)c;
                                    if(b->right==0)
                                    {
                                        printf(1,"Syntax Error\n");
                                        exit(-1);
                                    }
                                    else if(b->right->type == EXEC)
                                    {
                                        itr++;
                                        if(isAlphaNum(t->list[itr][0]) && itr<t->len)
                                        {                                               
                                            b->right = redircmd(b->right,0, &(t->list[itr][0]));
                                        }
                                        else
                                        {
                                            printf(1,"Missing File for Input\n");
                                            exit(0);
                                        }
                                    }
                                    else if(b->right->type == REDIR)
                                    {
                                        itr++;
                                        if(isAlphaNum(t->list[itr][0]) && itr<t->len)
                                        {
                                            r = (struct redircmd *) b->right;
                                            r->ofile = &(t->list[itr][0]);
                                        }
                                        else
                                        {
                                            printf(1,"Missing File for Input\n");
                                            exit(0);
                                        }
                                    }
                                    break;
                                }
                                case EXEC:
                                {
                                    itr++;
                                    if(isAlphaNum(t->list[itr][0]) && itr<t->len)
                                    {                                               
                                        c = redircmd(c,0, &(t->list[itr][0]));
                                        fcmd = c;
                                    }
                                    else
                                    {
                                        printf(1,"Missing File for Input\n");
                                        exit(0);
                                    }

                                    break;
                                }
                                case REDIR:
                                {
                                    itr++;
                                    if(isAlphaNum(t->list[itr][0]) && itr<t->len)
                                    {
                                        r = (struct redircmd *)c;
                                        r->ofile = &(t->list[itr][0]);
                                        fcmd = (struct cmd *)r;
                                    }
                                    else
                                    {
                                        printf(1,"Missing File for Input\n");
                                        exit(0);
                                    }
                                    break;
                                }
                                default:
                                    printf(1,"Error in Token Parser\n");
                                    break;
                            }
                        }   
                        else
                        {
                            printf(1,"Illegal Command");
                            exit(-1);
                        }
                        break;
                    }
            case '<':/*Read the file from next token*/
                    {
                        if(fcmd==0)
                        {printf(1,"Illegal command\n");exit(0);}

                        if(strlen(t->list[itr])==1)
                        {
                            struct cmd * c = getLastCmd(fcmd);
                            struct branchcmd *b;
                            switch(c->type)
                            {
                                case LIST:
                                case PIPE:
                                case COND:
                                {
                                    b = (struct branchcmd *)c;
                                    if(b->right==0 || b->right->type==REDIR)
                                    {
                                        printf(1,"Syntax Error\n");
                                        exit(-1);
                                    }
                                    else if(b->type == PIPE)
                                    {
                                          printf(1,"pipe input overriden by redirect input ERROR\n");
                                          exit(-1);  
                                    }
                                    else
                                    {
                                        itr++;
                                        if(isAlphaNum(t->list[itr][0]) && itr<t->len)
                                        {                                               
                                            b->right = redircmd(b->right, &(t->list[itr][0]),0);
                                        }
                                        else
                                        {
                                            printf(1,"Missing File for Input\n");
                                            exit(0);
                                        }
                                    }
                                    break;
                                }
                                case REDIR:
                                {
                                    printf(1,"Syntax Error\n");
                                    exit(-1);
                                }
                                case EXEC:
                                {
                                    itr++;
                                    if(isAlphaNum(t->list[itr][0]) && itr<t->len)
                                    {                                               
                                        c = redircmd(c, &(t->list[itr][0]),0);
                                        fcmd = c;
                                    }
                                    else
                                    {
                                        printf(1,"Missing File for Input\n");
                                        exit(0);
                                    }
                                    break;
                                }
                                default:
                                    printf(1,"Error in Token Parser\n");
                                    break;
                            }
                        }   
                        else
                        {
                            printf(1,"Illegal Command");
                            exit(-1);
                        }
                        break;
                    }
            default:
                    {
                        struct execcmd *e;
                        struct branchcmd *b;    
                        e= (struct execcmd *)execcmd();
                        int arg_itr=0;
                        while(isAlphaNum(t->list[itr][0]) && itr<t->len)
                        {
                            e->argv[arg_itr]=&(t->list[itr][0]);
                            itr++;
                            arg_itr++;
                        }
                        itr--;
                        switch(fcmd->type)
                        {
                            default:
                            {
                                fcmd = (struct cmd *)e;
                                break;                
                            }
                            case COND:
                            case PIPE:
                            case LIST:
                            {
                                struct cmd * c = getLastCmd(fcmd);
                                switch(c->type){
                                    case LIST:
                                    case PIPE:
                                    case COND:
                                        b = (struct branchcmd *)c;
                                        b->right=(struct cmd *)e;
                                        break;    
                                    default:
                                        break;
                                }
                                break;
                            }
                            case REDIR:
                            case EXEC:
                                printf(1,"Illegal systax\n");
                                break;
                        }   
                        break;
                    }
        }
    }
    printf(1,"fcmd->type = %d\n",fcmd->type);
    return fcmd;
}

void 
runcmd(struct cmd * fcmd)
{
    if(fcmd == 0)
        exit(0);

    switch(fcmd->type)
    {
        case COND:
        {
            struct condcmd * co = (struct condcmd *)fcmd;
            int tpid=0,cpid1=0,cpid2=0,cs1=-1,cs2=-1;
            if((cpid1=fork())==0)
            {
                runcmd(co->left);
            }
            printf(1,"pid %d forked\n",cpid1);
            tpid=wait(&cs1);
            printf(1,"pid %d status %d\n",tpid,cs1);
            if((co->condition == AND && cs1==0) || (co->condition == OR && cs1==-1))
            {   
                if((cpid2=fork())==0)
                {
                    runcmd(co->right);
                }
                printf(1,"pid %d forked\n",cpid2);
                tpid=wait(&cs2);
                printf(1,"pid %d status %d\n",tpid,cs2);
            }
            break;
        }
        case PIPE:
        {
            struct pipecmd * p = (struct pipecmd *)fcmd;
            int pio[2];
            if(pipe(pio)<0)
            {
                printf(2,"pipe creation failed\n");
                exit(-1);
            }
            else
            {
                /*redirect write of left to pipe 1*/
                if(fork()==0)
                {   
                    close(1);
                    dup(pio[1]);
                    close(pio[0]);
                    close(pio[1]);
                    runcmd(p->left);
                }
                /*redirect read of right to pipe 0*/
                else if(fork()==0)
                {
                    close(0);
                    dup(pio[0]);
                    close(pio[0]);
                    close(pio[1]);
                    runcmd(p->right);
                }
                close(pio[0]);
                close(pio[1]);
                wait(0);
                wait(0);
                break;
            }
        }
        case LIST:
        {
           struct listcmd * l = (struct listcmd *)fcmd;
           int cpid1,cpid2,cstatus1=-1,cstatus2=-1;
           
            if((cpid1=fork()) == 0)
            {
                runcmd(l->left);
            }
            else if((cpid2=fork()) ==0)
            {    
                runcmd(l->right);
            }   
            else
            {
                printf(1,"runCmd : p1 %d forked \t p2 %d forked \n",cpid1,cpid2);
                int teststatus1,teststatus2;
                int testpid1 = wait(&teststatus1);
                int testpid2 = wait(&teststatus2);
                if(testpid1 == cpid1)     
                {
                    cstatus1 = teststatus1;
                    cstatus2 = teststatus2;
                }
                else if(testpid2 == cpid1)
                {
                    cstatus1 = teststatus2;
                    cstatus2 = teststatus1;
                }
                printf(1,"p1 %d status : %d \t p2  %d status: %d\n",cpid1,cstatus1,cpid2,cstatus2);
            }
            break;
        }    
        case REDIR:
        {
            struct redircmd * r = (struct redircmd *)fcmd;
            if(r->ofile!=0)
            {
                close(r->ofd);
                int ofid = open(r->ofile,r->omode);
                if(ofid < 0){printf(2, "open %s failed\n", r->ofile);exit(-1);}
                printf(2,"Ofid : %d \n",ofid);
            }
            if(r->ifile!=0)
            {
                close(r->ifd);
                int ifid = open(r->ifile,r->imode);
                if(ifid < 0){printf(2, "open %s failed\n", r->ifile);exit(-1);}
                printf(2,"ifid : %d \n",ifid);
            }
            runcmd(r->cmd);
            break;
        }
        case EXEC:
        {
            struct execcmd * e = (struct execcmd *)fcmd;
            if(e->argv[0] == 0)
                exit(-1);
            printf(1,"%s %s\n",e->argv[0],e->argv[1]);
            exec(e->argv[0],e->argv);
            printf(2, "exec %s failed\n", e->argv[0]);
            break;
        }
        default:
        {
            panic("Error in run");
            break;    
        }
    }  
    free(fcmd);  
    exit(0);
}
