#! /bin/bash 

gawk $1 '
function capa(temp, SUF){
  st= -1;
  net = temp "/net.net"
  auxnet = temp "/aux.net"
  while (getline lin < net ) {
    N=split(lin,dades)
    gsub("W=","",dades[2])
    gsub("E=","",dades[3])
    if (N==2 && dades[2] == SUF) {
         gsub("I=","",dades[1]);
         st=dades[1]
         print lin > auxnet;
    } else if(N==4 && dades[3] == st){
         print dades[1],dades[2],"E="dades[3], "l=-32768"  >  auxnet;
    }  else print lin  >  auxnet;

  }
  close(auxnet)
  close(net)
  system("cp " auxnet " " net);
}

function mktemp() {
  _cmd = "mktemp -d /tmp/recognize_online.XXXXXX"
  _cmd | getline _tmpfile;
  close(_cmd)
  return _tmpfile
}


BEGIN {
  old_IFS = FS;
  FS = "=";

  while (getline < config) {
    gsub("^[ \t]*","",$1);
    gsub("^[ \t]*","",$2);
    gsub("[ \t]*$","",$1);
    gsub("[ \t]*$","",$2);
    
    conf[$1] = $2;
  }
  
  close(config);
  FS = old_IFS;
  temp = mktemp();
}

{
   gsub(","," , ",$0);
   gsub(";"," ; ",$0);

   i=1
   trans="";
   while(i <= NF && $i != ";"){
     if($i~"\"") $i="\\\"";
     trans=trans$i" ";
     i++;
   }

   if (i > 1) last_word = $(i-1);
   else last_word = "";
   if(last_word~"\"") {last_word="\\\\";last_word=last_word"\\\""; }

   i++;
   banned = $i;
   if (banned == ";") banned = "";
   while(i <= NF && $i != ";")  i++;

   num_strokes=0;
   while(i <= NF){
     i++;
     num_strokes++;
     num_puntos=0;
     while(i <= NF && $i!=";"){
       if($i==",") i++;
       num_puntos++;
       stroke[num_strokes,num_puntos]=$i" "$(i+1);
       i=i+2;
     }
     puntos[num_strokes]=num_puntos;
  }

  print "label" > temp "/muestra"
  print num_strokes > temp "/muestra" 
  for (i=1; i<=num_strokes;i++) {
     print puntos[i] > temp "/muestra"
     print "1" > temp "/muestra"
     for(j=1;j<=puntos[i];j++) {
        print stroke[i,j] > temp "/muestra"
     }
  }
  close(temp "/muestra");

  system(conf["get_OnLine_Feat"] " -o " temp "/muestra -n 0")
  
  if(last_word != "") {
    cmd=sprintf(conf["grammarTools"] " -i %s -s \"<s>\" \"</s>\" -p \"%s\" -n -o " temp "/net.net", conf["language_model"], last_word);
  } else {
	  cmd=sprintf(conf["grammarTools"] " -i %s -s \"<s>\" \"</s>\" -n -o " temp "/net.net", conf["language_model"]);
  }
  system(cmd);
  close(cmd);
  
  if (banned != "") {
    capa(temp, banned);
  }
  
  cmdRec=sprintf(conf["HVite"] " -A -T 1 -C " conf["config_ATROS"] " -o T -s 10 -H " conf["Macros_hmm"] " -t " conf["beam_t"] " -u " conf["beam_u"] " -w " temp "/net.net %s " conf["simbolos"] " " temp "/muestra_1.fea 2> /dev/null | tail -1", conf["dictionary"]);
  cmdRec | getline result
  close(cmdRec);
  gsub("<s> ", "", result);
  gsub(" </s>.*", "", result);

  system("rm -r " temp)

  printf result
}
'
