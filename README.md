# Server-ex-select
Yaron Mishaly
Server program  that handle multiple client using select


This program is a server that handle multiple client using select() 
The client send the program he wish to preform and the server creats a child procces that preform the task using the exec system call.
The child procces and the parent procces communicate via a pipe 

ירון משאלי 
פרויקט סרבר: 
 Server ex-select  
נכתב בשפת C  , במערכת לינוקס, 
המטפל במספר לקוחות באמצעות select( ). 
מקבל: שם של תוכנית להרצה ויוצר תהליך-בן שמריץ אותה, 
ע"י קריאת מערכת  exec(). 
הסרבר מחזיר ללקוח את הפלט של התוכנית. 
תהליך-הבן ותהליך-האב מקושרים באמצעות pipe




