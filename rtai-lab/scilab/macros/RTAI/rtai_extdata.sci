function [x,y,typ]=rtai_extdata(job,arg1,arg2)
//
// Copyright roberto.bucher@supsi.ch
x=[];y=[];typ=[];
select job
case 'plot' then
  graphics=arg1.graphics; exprs=graphics.exprs;
  filename=exprs(1)(1);
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
  while %t do
    [ok,filename,npts,lab]=..
        getvalue('Set extdata data block parameters',..
        ['Filename';
	'Points'],..
         list('str',1,'vec',1),label(1))

    if ~ok then break,end
    label(1)=lab
    name=strsubst(filename,'.','_');
    funam='i_extdata_' + name;
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

    [ok,tt]=getCode(funam)
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
  npts=1000;
  filename='test.dat'

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

  label=list([filename,sci2exp(npts)],[])

  gr_i=['xstringb(orig(1),orig(2),[''extdata'';filename],sz(1),sz(2),''fill'');']
  x=standard_define([3 2],model,label,gr_i)

end
endfunction

function [ok,tt]=getCode(funam)
   textmp=[
          '#ifndef MODEL'
          '#include <math.h>';
          '#include <stdlib.h>';
          '#include <scicos/scicos_block.h>';
          '#endif'
          '';
          'void '+funam+'(scicos_block *block,int flag)';
         ];
  textmp($+1)='{'
  textmp($+1)='#ifdef MODEL'
  textmp($+1)='static int port;'
  textmp($+1)='double y[1];'
  textmp($+1)='double t = get_scicos_time();' 
  textmp($+1)='  switch(flag) {'
  textmp($+1)='  case 4:'
  textmp($+1)='    port=inp_extdata_init('+ string(npts)+',""' + filename + '"");'
  textmp($+1)='    break;'; 
  textmp($+1)='  case 1:'
  textmp($+1)='    inp_extdata_input(port,y,t);'
  textmp($+1)='    block->outptr[0][0]=y[0];'
  textmp($+1)='    break;'
  textmp($+1)='  case 5:'
  textmp($+1)='    inp_extdata_end(port);'
  textmp($+1)='    break;'
  textmp($+1)='  }'
  textmp($+1)='#endif'
  textmp($+1)='}'
  tt=textmp;

  ok = %t;

endfunction
