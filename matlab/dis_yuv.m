function disp_yuv(width, height, format) 
% DISP_YUV  Display YUV422 image 
% 
% width     The width of the image 
% height    The height of the image 
% format    YUV422 format, 0 for YUYV, 1 for UYVY 
 
 
fid = fopen('test.yuv', 'r'); 
yuv = fread(fid); 
fclose(fid); 
 
if(format == 0) 
    yy = yuv(1:2:width.*height.*2); 
    uu(1:2:width.*height) = yuv(2:4:width.*height.*2); 
    uu(2:2:width.*height) = yuv(2:4:width.*height.*2); 
    vv(1:2:width.*height) = yuv(4:4:width.*height.*2); 
    vv(2:2:width.*height) = yuv(4:4:width.*height.*2); 
elseif(format == 1) 
    yy = yuv(2:2:width.*height.*2); 
    uu(1:2:width.*height) = yuv(1:4:width.*height.*2); 
    uu(2:2:width.*height) = yuv(1:4:width.*height.*2); 
    vv(1:2:width.*height) = yuv(3:4:width.*height.*2); 
    vv(2:2:width.*height) = yuv(3:4:width.*height.*2); 
end 
     
yyuuvv = [yy uu' vv']/255; 
rrggbb = ycbcr2rgb(yyuuvv); 
 
fid1 = fopen('temp.dat', 'w'); 
fwrite(fid1, yy, 'uint8'); 
fclose(fid1); 
fid1 = fopen('temp.dat', 'r'); 
yyy = fread(fid1, [width height]); 
fclose(fid1); 
yyy = yyy'/255; 
figure 
imshow(yyy) 
 
fid1 = fopen('temp.dat', 'w'); 
fwrite(fid1, rrggbb(:, 1), 'double'); 
fclose(fid1); 
fid1 = fopen('temp.dat', 'r'); 
rrr = fread(fid1, [width height], 'double'); 
fclose(fid1); 
 
fid1 = fopen('temp.dat', 'w'); 
fwrite(fid1, rrggbb(:, 2), 'double'); 
fclose(fid1); 
fid1 = fopen('temp.dat', 'r'); 
ggg = fread(fid1, [width height], 'double'); 
fclose(fid1); 
 
fid1 = fopen('temp.dat', 'w'); 
fwrite(fid1, rrggbb(:, 3), 'double'); 
fclose(fid1); 
fid1 = fopen('temp.dat', 'r'); 
bbb = fread(fid1, [width height], 'double'); 
fclose(fid1); 
 
rgb = cat(3, rrr', ggg', bbb'); 
 
figure 
imshow(rgb)

