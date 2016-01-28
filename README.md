###rash --  Ridiculously Awesome Shell       

#####Include:       
1. Builtins:            
   Type ``rashhelp'' to  see all builtin commands.                   
   * export -- export environment variables.          
   * cd  -- change current working directory.            
   * pwd -- print current working directory.          
   * rashenv -- print current shell's variables.         
   * umask -- set or get umask.        
   * exec -- execute script file ( inline commands ).             
2. Lists:         
   Pattern: cmd; cmd; cmd;              
3. Redirection: 
   Supported work with standard file descriptors.       
   * N>[&fd|filename] where N is either stderr ( N = 2) or stdin ( N='').                  
   * N<[filename] where N is stdin.       
4. Environment:            
   Supported two type of variables:              
   * Environment variables is global variables inherited by child processes.           
     > export NAME=VAR               
   * Shell's local variables are available in the current shell.          
     > NAME=VAR      

   Special variables:
      * PROMPT -- set prompt.         
      * $ -- current shell's pid.            
      * ? -- last exit value.                

   rash reads config file from the path $HOME/rashrc.
