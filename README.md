# mtcnn
基于caffe的mtcnn训练实现，非常容易非常简单，并且有配套的纯c++版本的mtcnn-light<br/>
<br/>
### caffemodel_2_mtcnnmodel: 
caffemodel文件转换为mtcnn-light支持的.h头文件类型的模型，可以直接丢进去替换编译就行了<br/>
### mtcnn-light: 
改进版本的mtcnn-light，配合训练程序，和转换程序，实现一整套的流程到最后部署<br/>
### train: 
训练过程所使用的程序和脚本<br/>
<br/>
<br/>

## 训练
1.基础问题<br/>
a.样本问题，mtcnn训练时，会把训练的原图样本，通过目标所在区域进行裁剪，得到三类训练样本，即：正样本、负样本、部分(part)样本<br/>
  其中：<br/>
  裁剪方式：对目标区域，做平移、缩放等变换得到裁剪区域<br/>
  IoU：目标区域和裁剪区域的重合度<br/>
  <br/>
  此时三类样本如下定义：<br/>
  正样本：IoU >= 0.65，标签为1<br/>
  负样本：IoU < 0.3，标签为0<br/>
  部分(part)样本：0.65 > IoU >= 0.4，标签为-1<br/>
<br/>
b.网络问题，mtcnn分为三个小网络，分别是PNet、RNet、ONet，新版多了一个关键点回归的Net（这个不谈）。<br/>
  PNet：12*12，负责粗选得到候选框，功能有：分类、回归<br/>
  RNet：24*24，负责筛选PNet的粗筛结果，并微调box使得更加准确和过滤虚警，功能有：分类、回归<br/>
  ONet：48*48，负责最后的筛选判定，并微调box，回归得到keypoint的位置，功能有：分类、回归、关键点<br/>
<br/>
c.网络大小的问题，训练时输入图像大小为网络指定的大小，例如12*12，而因为PNet没有全连接层，是全卷积的网络，所以预测识别的时候是没有尺寸要求的，那么PNet可以对任意输入尺寸进行预测得到k个boundingbox和置信度，通过阈值过滤即可完成候选框提取过程，而该网络因为结构小，所以效率非常高。<br/>
<br/><br/>
2.训练步骤<br/>
a.准备好要训练的样本图片放到train目录下，以及对应的train/label.txt文件，格式是：<br/>
  filename xmin ymin xmax ymax point1x point1y point2x point2y point3x point3y point4x point4y<br/>
  因为默认设置的是4个关键点加上boundingbox，如果不需要关键点，响应的去掉就好了，然后修改对应的脚本程序和去掉对应的网络层即可<br/>
b.双击gen-data12.bat，制作12*12网络PNet的训练样本，样本存放在当前目录12目录下<br/>
c.双击gen_12net_list.bat，对12目录下的样本图片和txt文件，综合为一个label-train.txt文件，格式是提供给convert_imageset.exe使用<br/>
d.双击make-lmdb-12.bat，创建lmdb数据库<br/>
e.双击train-12.bat，开始对12*12网络做训练<br/>
f.一般训练几万次后，loss到0.0x的时候就可以接受了<br/>
<br/>
记得在当前目录下创建models-12、models-24、models-48来迎接喜气招财哟~<br/>
<br/>
3.使用阶段<br/>
将训练的caffemodel，复制到caffemodel_2_mtcnnmodel里面，编译执行他（代码写的必须3个网络同时存在，所以自己看情况改下），这时候产生的mtcnn_models.h，就是我们要的网络头文件，添加到mtcnn-light覆盖下就可以执行看效果了<br/>

<br/>
<br/>
还在完善中...加油<br/>