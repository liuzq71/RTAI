function [x,y,typ]=rtai_pcan_in(job,arg1,arg2)
//
// Copyright roberto.bucher@supsi.ch
x=[];y=[];typ=[];
select job
case 'plot' then
  standard_draw(arg1)
case 'getinputs' then
  [x,y,typ]=standard_inputs(arg1)
case 'getoutputs' then
  [x,y,typ]=standard_outputs(arg1)
case 'getorigin' then
  [x,y]=standard_origin(arg1)
case 'set' then
  x=arg1
  model=arg1.model;graphics=arg1.graphics;
  label=graphics.exprs;
  oldlb=label(1)
  while %t do
    [ok,port,pcan_id,kp,ki,typ,lab]=..
        getvalue('Set PCAN data block parameters',..
        ['Port';
        'CAN Id';
	'Proportional Gain';
        'Integral Gain';
	'Typ'],...
         list('vec',1,'str',1,'vec',1,'vec',1','str',1),label(1))

    if ~ok then break,end
    label(1)=lab
    funam='i_pcan_in_' + string(port);
    xx=[];ng=[];z=0;
    nx=0;nz=0;
    i=[];
    o=1;nout=1;
    ci=1;nevin=1;
    co=[];nevout=0;
    funtyp=2004;
    depu=%t;
    dept=%f;
    dep_ut=[depu dept];
    ntyp=0;

    if grep(typ,'EPP') | grep(typ,'epp') then
      ntyp=1;
    elseif grep(typ,'ISA') | grep(typ,'isa') then
      ntyp=2;
    else
      ntyp=0;
    end

    tt=label(2);
    if find(oldlb <> label(1)) <> [] then
      tt=[]
    end

    [ok,tt]=getCode(funam,tt)
    if ~ok then break,end
    [model,graphics,ok]=check_io(model,graphics,i,o,ci,co)
    if ok then
      model.sim=list(funam,funtyp)
      model.in=[]
      model.out=o
      model.evtin=ci
      model.evtout=[]
      model.state=[]
      model.dstate=0
      model.rpar=[]
      model.ipar=[]
      model.firing=[]
      model.dep_ut=dep_ut
      model.nzcross=0
      label(2)=tt
      x.model=model
      graphics.exprs=label
      x.graphics=graphics
      break
    end
  end
case 'define' then
  port=1;kp=8000;ki=4000; 
  rt_par=[kp,ki,0,0,0]
  rpar=rt_par(:)
  pcan_id='0x601'
  typ='EPP';
  ntyp=1;

  model=scicos_model()
  model.sim=list(' ',2004)
  model.in=[]
  model.out=1
  model.evtin=1
  model.evtout=[]
  model.state=[]
  model.dstate=[]
  model.rpar=[]
  model.ipar=[]
  model.blocktype='c'
  model.firing=[]
  model.dep_ut=[%t %f]
  model.nzcross=0

  label=list([sci2exp(port),pcan_id,sci2exp(kp),sci2exp(ki),typ],[])

  gr_i=['xstringb(orig(1),orig(2),''PCAN IN'',sz(1),sz(2),''fill'');']
  x=standard_define([2 2],model,label,gr_i)

end
endfunction

function [ok,tt]=getCode(funam,tt)
//
if tt==[] then
  
  textmp=[
	 '#ifndef MODEL'
	 '#include <math.h>';
	 '#include <stdlib.h>';
	 '#include <scicos/scicos_block.h>';
	 '#endif'
	 'void '+funam+'(scicos_block *block,int flag)';
	 ];
  	 ttext=[];
  	 textmp($+1)='{'
  
  	 textmp($+1)='  switch(flag) {'
  	 textmp($+1)='  case 4:'
  	 textmp($+1)='   '+funam+"_bloc_init(block,flag);"
  	 textmp($+1)='   break;';
    	 l1 = '  inp_pcan_init(' + string(port) + ',0,';
    	 l2 = '""' + pcan_id + '"","""",' + string(kp) + ',' + string(ki) + ',' + string(ntyp) + ',0,0';
    	 ttext=[ttext;'int '+funam+"_bloc_init(scicos_block *block,int flag)";
         '{';
         '#ifdef MODEL'
         l1 + l2 + ');';
         '#endif'
         '  return 0;';
         '}'];
  	 textmp($+1)=' '

  	 textmp($+1)='  case 1:'
   	 textmp($+1)='   set_block_error('+funam+"_bloc_outputs(block,flag));"
  	 textmp($+1)='   break;';
  	 ttext=[ttext;'int '+funam+"_bloc_outputs(scicos_block *block,int flag)";
         "{";
         "  double y[1];";
         "  double t = get_scicos_time();";
         '#ifdef MODEL'
         "  inp_pcan_input(" + string(port) + ",y,t);";
         '#endif'
         "  block->outptr[0][0]=y[0];";
         "  return 0;";
         "}"];
 	 textmp($+1)='  case 5: '
      	 textmp($+1)='     set_block_error('+funam+"_bloc_ending(block,flag));";
      	 textmp($+1)='   break;';
         ttext=[ttext;'int '+funam+"_bloc_ending(scicos_block *block,int flag)";
         "{";
         '#ifdef MODEL'
         "  inp_pcan_end(" + string(port) + ");";
         '#endif'
         "  return 0;";
         "}"];
  	 textmp($+1)='  }'
 	 textmp($+1)='}'
	 textmp=[textmp;' '; ttext];
else
  textmp=tt;
end

while 1==1
  [txt]=x_dialog(['Function definition in C';
		  'Here is a skeleton of the functions which you should edit'],..
		 textmp);
  
  if txt<>[] then
    tt=txt
    textmp=txt;
    break;
  end
end

endfunction
