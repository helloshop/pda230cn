
% Actual temperature
file = fopen('col_5.txt','r');
if file ~= -1
    temp_actual = fscanf(file, '%d');
    fclose(file);
end

file = fopen('col_6.txt','r');
if file ~= -1
    p_term = fscanf(file, '%d');
    fclose(file);
end

file = fopen('col_7.txt','r');
if file ~= -1
    d_term = fscanf(file, '%d');
    fclose(file);
end

plot(temp_actual,'r');
hold on;

plot(p_term,'b');
hold on;

plot(d_term,'g');
hold on;


grid on;
title('Temperature setting');
xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










