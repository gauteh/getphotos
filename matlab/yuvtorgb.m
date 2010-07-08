function [a] = yuvtorgb(no)
    
  %Velger YUV 420 format, er det som funka.
  fwidth = 0.5;
  fheight= 0.5;

  width = 720;
  height = 480;

  for k = 1:1:no
    file = sprintf ('%d.yuv420', k); 
    disp (sprintf ('On image %d of %d', k, no))
    YUV = loadFileYUV(width,height,file,fheight,fwidth);
    RGB = ycbcr2rgb(YUV); %Convert YUV to RGB

    % write
    out = sprintf('../../%d.bmp', k);
    imwrite (RGB, out, 'bmp');
  end

end


% loadFileYUV(width,height,File,fheight,fwidth)
function YUV = loadFileYUV(width,heigth,fileName,Teil_h,Teil_b)
  % get size of U and V
  fileId = fopen(fileName,'r');
  width_h = width*Teil_b;
  heigth_h = heigth*Teil_h;

  % create Y-Matrix
  YMatrix = fread(fileId, width * heigth, 'uint8');
  YMatrix = reshape(YMatrix,width,heigth)';

  % create U- and V- Matrix
  UMatrix = fread(fileId,width_h * heigth_h, 'uint8');
  UMatrix = reshape(UMatrix,width_h, heigth_h).';

  VMatrix = fread(fileId,width_h * heigth_h, 'uint8');
  VMatrix = reshape(VMatrix,width_h, heigth_h).';

  %Show U- and V- matrix
  %figure; imshow(UMatrix,[]);
  %figure; imshow(VMatrix,[]);

  % consideration of the subsampling of U and V
  % UMatrix1(1:heigth_h,1:width) = 0;
  % UMatrix1(1:heigth_h,1:2:end) = UMatrix(:,1:1:end);
  % UMatrix1(1:heigth_h,2:2:end) = UMatrix(:,1:1:end);
  UMatrix1 = imresize (UMatrix, [heigth width]);

  %VMatrix1(1:heigth_h,1:width) = 0;
  %VMatrix1(1:heigth_h,1:2:end) = VMatrix(:,1:1:end);
  %VMatrix1(1:heigth_h,2:2:end) = VMatrix(:,1:1:end);
  VMatrix1 = imresize (VMatrix, [heigth width]);


  % compose the YUV-matrix:
  YUV(1:heigth,1:width,1) = YMatrix;

  %YUV(1:heigth,1:width,2) = 0;
  %YUV(1:2:end,:,2) = UMatrix1(:,:);
  %YUV(2:2:end,:,2) = UMatrix1(:,:);
  YUV(:,:,2) = UMatrix1;

  %YUV(1:heigth,1:width,3) = 0;
  %YUV(1:2:end,:,3) = VMatrix1(:,:);
  %YUV(2:2:end,:,3) = VMatrix1(:,:);
  YUV(:,:,3) = VMatrix1;

  YUV = uint8(YUV);

  fclose(fileId);
end
