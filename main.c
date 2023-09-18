#include "parser.h"
#include <sys/wait.h>
#include <signal.h>

int background_pids[64];
int num_backgrounds = 0;
int foreground_pid;

void printcmd(struct cmd *cmd)
{
    struct backcmd *bcmd = NULL;
    struct execcmd *ecmd = NULL;
    struct listcmd *lcmd = NULL;
    struct pipecmd *pcmd = NULL;
    struct redircmd *rcmd = NULL;

    int i = 0;
    
    if(cmd == NULL)
    {
        PANIC("NULL addr!");
        return;
    }
    

    switch(cmd->type){
        case EXEC:
            ecmd = (struct execcmd*)cmd;
            if(ecmd->argv[0] == 0)
            {
                goto printcmd_exit;
            }

            MSG("COMMAND: %s", ecmd->argv[0]);
            for (i = 1; i < MAXARGS; i++)
            {            
                if (ecmd->argv[i] != NULL)
                {
                    MSG(", arg-%d: %s", i, ecmd->argv[i]);
                }
            }
            MSG("\n");

            break;

        case REDIR:
            rcmd = (struct redircmd*)cmd;

            printcmd(rcmd->cmd);

            if (0 == rcmd->fd_to_close)
            {
                MSG("... input of the above command will be redirected from file \"%s\". \n", rcmd->file);
            }
            else if (1 == rcmd->fd_to_close)
            {
                MSG("... output of the above command will be redirected to file \"%s\". \n", rcmd->file);
            }
            else
            {
                PANIC("");
            }

            break;

        case LIST:
            lcmd = (struct listcmd*)cmd;

            printcmd(lcmd->left);
            MSG("\n\n");
            printcmd(lcmd->right);
            
            break;

        case PIPE:
            pcmd = (struct pipecmd*)cmd;

            printcmd(pcmd->left);
            MSG("... output of the above command will be redirected to serve as the input of the following command ...\n");            
            printcmd(pcmd->right);

            break;

        case BACK:
            bcmd = (struct backcmd*)cmd;

            printcmd(bcmd->cmd);
            MSG("... the above command will be executed in background. \n");    

            break;


        default:
            PANIC("");
    
    }
    
    printcmd_exit:

    return;
}

void runcmd(struct cmd *cmd, int background)
{
    struct backcmd *bcmd = NULL;
    struct execcmd *ecmd = NULL;
    struct listcmd *lcmd = NULL;
    struct pipecmd *pcmd = NULL;
    struct redircmd *rcmd = NULL;

    int status, i;

    if(cmd == NULL)
    {
        PANIC("NULL addr!");
        return;
    }

    for (i = 0; i < num_backgrounds; i++) {
        if (background_pids[i] == 0) {
            continue;
        }
        if (waitpid(background_pids[i], &status, WNOHANG) == background_pids[i]) {
            int j;
            for (j = 0; j < num_backgrounds; j++) {
                if (background_pids[j] == background_pids[i]) {
                    background_pids[j] = 0;
                }
            }
            if (status != 0) {
                printf("Non-zero exit code (%d) detected\n", status);
            }
        }
    }

    switch(cmd->type){
        case EXEC:
            ecmd = (struct execcmd*)cmd;
            if (background) {
                execvp(ecmd->argv[0], &ecmd->argv[0]);
                exit(0);
            } else {
                int child = fork();
                foreground_pid = child;
                if (child == 0) {
                    execvp(ecmd->argv[0], &ecmd->argv[0]);
                    exit(0);
                } else {
                    waitpid(child, &status,0);
                    status = WEXITSTATUS(status);
                    if (status != 0) {
                        printf("Non-zero exit code (%d) detected\n", status);
                    }
                    foreground_pid = 0;
                }
            }

            break;

        case REDIR:
            rcmd = (struct redircmd*)cmd;
            int redir = fork(), fd;

            if (redir == 0) {
                if (rcmd->mode == O_RDONLY) {
                    fd = open(rcmd->file, O_RDONLY, 0);
                    dup2(fd, STDIN_FILENO);
                } else {
                    fd = open(rcmd->file, O_CREAT|O_RDWR|O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
                }
                close(fd);
                runcmd(rcmd->cmd,0);
                exit(0);
            } else {
                wait(NULL);
            }

            break;

        case LIST:
            lcmd = (struct listcmd*)cmd;

            runcmd(lcmd->left,0);
            runcmd(lcmd->right,0);
            
            break;

        case PIPE:
            pcmd = (struct pipecmd*)cmd;
            int pfds[2];
            int children[2];
            pipe(pfds);

            children[0] = fork();
            if (children[0] == 0) {
                dup2(pfds[1],1);
                close(pfds[0]);
                close(pfds[1]);
                runcmd(pcmd->left,1);
                exit(0);
            }

            children[1] = fork();
            if (children[1] == 0) {
                dup2(pfds[0],0);
                close(pfds[0]);
                close(pfds[1]);
                runcmd(pcmd->right,1);
                exit(0);
            }
            close(pfds[0]);
            close(pfds[1]);

            int num_children = 2;
            while (num_children) {
                wait(NULL);
                num_children--;
            }

            break;

        case BACK:
            bcmd = (struct backcmd*)cmd;

            int back = fork();
            if (back == 0) {
                runcmd(bcmd->cmd, 1);
                exit(0);
            } else {
                background_pids[num_backgrounds] = back;
                num_backgrounds++;
            }

            break;

        default:
            PANIC("");
    
    }

    return;
}

void interrupt(int signal) {
    if (SIGINT == signal) {
        if (foreground_pid) {
            kill(foreground_pid, SIGINT);
        } else {
            printf("\nCtrl-C caught. But currently there is no foreground process running.\n");
            printf("myshell> ");
        }
    }
}

int main(void)
{
    static char buf[1024];
    setbuf(stdout, NULL);
    signal(SIGINT, interrupt);

    while(getcmd(buf, sizeof(buf)) >= 0)
    {
        struct cmd * command;
        command = parsecmd(buf);
        runcmd(command, 0);
    }

    PANIC("getcmd error!\n");
    return 0;
}
