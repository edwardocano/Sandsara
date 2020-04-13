import processing.serial.*;
Serial mySerial;
long[] values; 
int picture_number = 0;

void data(long[] dvalor){
  String value = mySerial.readStringUntil(10);
  //print(value);
  if (value != null){
    if (value.length()>=8){
          if (value.substring(0,8).equals("finished")){
              background(0);
              /*pushMatrix();
              segment(x, y, coordinates[1]);
              segment(segLength, 0, coordinates[2]);
              popMatrix();*/
              translate(x , y);
              sandmark.beginDraw();
              sandmark.translate(x, y);
              sandmark.noStroke();
              sandmark.fill(0, 0, 255);
              sandmark.circle(x_draw, y_draw, 10);
              sandmark.endDraw();
              image(sandmark, -x, -y);
              noFill();
              strokeWeight(3);
              stroke(255);
              circle(0, 0, 304*factor);
              save(picture_number + ".jpg");
              println("guardo el archivo: " + picture_number);
              picture_number += 1;
              sandmark.clear();
              sandmark.beginDraw();
              sandmark.fill(abs(sin(picture_number*0.1)*255),abs(cos(picture_number*0.17)*255),
              abs(sin(picture_number*0.3)*cos(picture_number*0.2)*255));
              sandmark.endDraw();
                noStroke();
                fill(0, 0, 255);
                circle(x_draw, y_draw, 10);
                numberPoint = 0;
              dvalor[0] = 0;
              dvalor[1] = 0;
              dvalor[2] = 0;
              return;
            }
        }
      int index = value.indexOf(",");
      String a = value.substring(index+1, value.length()-2);
      int index2= a.indexOf(",");
      
      while(index <= 0 || index2 <= 0){
      value = mySerial.readStringUntil(10);
      index = value.indexOf(","); 
      a = value.substring(index+1, value.length()-2);
      index2= a.indexOf(",");
      }
      
      String xstr= value.substring(0, index);
      String ystr = a.substring(0, index2);
      String zstr = a.substring(index2+1);
      //String zstr = a.substring(index2+1,a.length()-1);
      long xlong = Long.parseLong(xstr);
      long ylong = Long.parseLong(ystr);
      long zlong = Long.parseLong(zstr);
      dvalor[0] = xlong;
      dvalor[1] = ylong;
      dvalor[2] = zlong;
      numberPoint += 1;
    }
    else {
      dvalor[0] = 0;
      dvalor[1] = 0;
      dvalor[2] = 0;}
}
