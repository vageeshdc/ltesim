sinrInt = sinrValues(:,2:4);
sinrInt = int32(sinrInt);

minx = min(sinrInt(:,1));
maxx = max(sinrInt(:,1));
maxy = max(sinrInt(:,2));
miny = min(sinrInt(:,2));

Xa = zeros((1+maxx-minx),1);
Ya = zeros((1+maxy-miny),1);
Za = zeros((1+maxx-minx),(1+maxy-miny));


for i=1:1+maxx-minx
Xa(i) = i+minx-1;
end

for i=1:1+maxy-miny
Ya(i) = i+miny-1;
end

%xar = imresize(Xa,[1,20]);
%yar = imresize(Ya,[1,20]);
%zar = imresize(Za,[20,20]);
%h = fspecial('gaussian',[15,15],5);
%Zar = filter2(h,Za);

%{
surfc(yar,zar,zar);
colormap hsv;

%[X,Y] = meshgrid([-2:.25:2]);
%Z = X.*exp(-X.^2-Y.^2);
contour3(Ya,Xa,Za)
surface(Ya,Xa,Za,'EdgeColor','FaceColor','none')
grid off
%view(-15,25)
colormap cool

%surf(Ya,Xa,Za);%best plot!!
%colormap(jet);
%}

numUE = 750;
maxZr = 10^(int32(log10((max(size(sinrInt))/numUE)*100)));

for j = 1:100:(max(size(sinrInt))/numUE)-1

    for i=1:numUE
    Za(sinrInt(i,1)-minx+1,sinrInt(i,2)-miny+1) = sinrInt(i+(j-1)*numUE,3);
    end

    xar = imresize(Xa,[1,100]);
    yar = imresize(Ya,[1,100]);
    zar = imresize(Za,[100,100]);
    
    %f = figure('visible','off'),contour3(Ya,Xa,Za,100);%best plot!!
    f = figure('visible','off'),contour3(yar,xar,zar.*100,100);%best plot!!
    %imgName = 'image_'j;
    fname = sprintf('series/myfileb%5d.jpeg', j+maxZr);
    print(f,'-djpeg',fname);
    %print(f, '-r80', '-dtiff', fname);
end
