clear all; close all; clc;
system('Virtuoso.exe &'); % Open up the C++ program

figure % Open figure for GUI

fid = fopen('data.txt'); % Open text file 

left = zeros(2,1);
right = zeros(2,1);

axis([0 1 0 1])
legend({'Right Hand', 'Left Hand'});
set(gca, 'xtick', [0 .4, .6 1]); % Set grid 
set(gca, 'ytick', [0 .3, .5 1]);
xlabel('Horizontal Direction')
ylabel('Vertical Direction')
title('Virtuoso User Hand Locations')
grid on
hold on

% Make initial points
plot(0, 0, 'ro', 'MarkerFaceColor', 'r', 'LineWidth', 1) 
hold on;
plot(0, 0, 'bo', 'MarkerFaceColor', 'b', 'LineWidth', 1)
led = legend({'Right Hand', 'Left Hand'});
pos = get(led, 'Position');
set(legend, 'Position', [0 .9 pos(3:end)])
hold off
h = get(gca,'Children');

i = 1;
fileLine  = fgets(fid);

while(~strcmp(get(gcf,'CurrentKey'),'return'))
        fileLine = fgets(fid);
        if fileLine  ~= -1
                   
                points = eval([ '[' strcat(fileLine ) ']' ]); % Parse text file
                if points(3) == 0
                    right = [points(1); points(2)];
                end
                if points(3) == 1
                    left = [points(1); points(2)];
                end
               
                set(h(1), 'XData', right(1)) % Set point positions
                set(h(1), 'YData', right(2))
                set(h(2), 'XData', left(1))
                set(h(2), 'YData', left(2))

         end
         pause(.0001); 
end

close all;
fclose(fid);