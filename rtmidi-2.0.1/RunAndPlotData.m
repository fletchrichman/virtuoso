clear all; close all; clc;
system('pleasework2.exe &');

figure

fid = fopen('data2.txt');

len = 100;
left = zeros(2,1);
right = zeros(2,1);

axis([0 1 0 1])
legend({'Right Hand', 'Left Hand'});
set(gca, 'xtick', [.33, .67]);
set(gca, 'ytick', [.33, .67]);
grid on
hold on

plot(0, 0, 'ro')
hold on;
plot(0, 0, 'bo')
hold off
h = get(gca,'Children');


i = 1;
x = fgets(fid);
while(~strcmp(get(gcf,'CurrentKey'),'return'))
        x = fgets(fid);
        if x ~= -1
                
                
                a = eval([ '[' strcat(x) ']' ]);
                if a(3) == 0
                    right = [a(1); a(2)];
                  
 
                end
                if a(3) == 1
                    left = [a(1); a(2)];
                end
               % axis([-100 100 -100 100])
               
               
                set(h(1), 'XData', right(1))
                set(h(1), 'YData', right(2))
                set(h(2), 'XData', left(1))
                set(h(2), 'YData', left(2))

         end
         pause(.0001);
    
end

close all;
fclose(fid);

