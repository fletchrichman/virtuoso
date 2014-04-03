clear all; close all; clc;

fid = fopen('data12.txt');

len = 100;

i = 1;
x = fgets(fid);
while(1)
        x = fgets(fid);
%         if x ~= -1
                a = eval([ '[' strcat(x) ']' ]);
                if a(3) == 0
                    scatter(a(1),a(2),5,'b');
                end
                if a(3) == 1
                    scatter(a(1),a(2),5,'r');
                end
               % axis([-100 100 -100 100])
               axis([0 1 0 1])
                hold on
                i = mod(i,len)+1;  
                if i == 1
                    clf
                end
                
%         end
         pause(.0001);
    
end