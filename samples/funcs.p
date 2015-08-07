program functions;
var a, b : integer;
	d : integer;
const c  = 150;
procedure forw; forward;

function test(a : integer; b : integer): integer;
var d : integer;
const c = 15;
begin
	writeln('In test\'s local scope');
	d := 20;
	forw();
	write('a: ');
	writeln(a);

	write('b: ');
	writeln(b);	

	write('c: ');
	writeln(c);

	write('d: ');
	writeln(d);			
end;

procedure forw;
begin
	writeln('Forward call');
end;

function plus(a : integer; b : integer): integer;
begin
	plus := a + b;
	exit;
end;

begin
	writeln('Local scope');
	test(5, 10);

	a := 50;
	b := 100;
	d := 200;

	writeln('Global scope');
	write('a: ');
	writeln(a);

	write('b: ');
	writeln(b);	

	write('c: ');
	writeln(c);

	write('d: ');
	writeln(d);			


	writeln('#####################');
	write('a + b:');	
	writeln( plus(a, b) );	

end.