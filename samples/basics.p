program preview;

var number, i : integer;
	arr : array [ -10 .. 10] of integer;
	arr2 : array [ 0 .. 10] of integer;
begin
	writeln('My pascal compiler features preview');
	write('Please enter number in range (0, 10) and press enter: ');
	read(number);
	write('Number you entered: ');
	writeln(number);

	writeln('#####################################');
	writeln('Lets try some cycles: ');
	write('Upto:   ');
	for i := 1 to number do
	begin
		write(i);
		write(' ');
	end;
	writeln('');
	write('Downto: ');

	for i := number downto 1 do
	begin
		write(i);
		write(' ');
	end;	
	writeln('');
	writeln('While cycle - enter 0 for break');

	number := 1;
	while number <> 0 do
	begin
		writeln('Enter number: ');
		read(number);
	end;
	writeln('#####################################');

	writeln('Nested blocks:');
	begin
		writeln('1. level');
		begin
			writeln('2. level');
			begin
				writeln('3. level');
				begin
					writeln('4. level');
					begin
						writeln('5. level');
						begin
							writeln('6. level');
						end;
					end;
				end;
			end;
		end;	
	end;
	
	writeln('#####################################');
	writeln('Array:');
	for i := -10 to (10 - 1) do
	begin
		arr[i] := i * 10;

		if i >= 0 then
		begin
			arr2[i] := (100 * i) / 33;
		end;
	end;

	for i := -10 to (8 + 1) do
		writeln(arr[i]);

	writeln('===================================');
	for i := -0 to (9 mod 10) do
		writeln(arr2[i]);		
	writeln('===================================');

	writeln('Break');

	for i := 0 to 10 do
	begin
		write('i = ');
		writeln(i);
		if i >= 2 then
		begin
			writeln('Break yet');
			break;
		end
		else
			writeln('Not yet break');
	end;

	writeln('#################################');
	write('5 * 4 + 3 = ');
	writeln( 5 * 4 + 3 );

	write('20 mod 3 + 4 = ');
	writeln( 20 mod 3 + 4 );

	write('&10: ');
	writeln( &10 );

	write('010: ');
	writeln( 010 );

	write('$10: ');
	writeln( $10 );

	write('0x10: ');
	writeln( 0x10 );			

end.