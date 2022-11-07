# generic_ghost_installer  

- [ch](#如何使用)
- [jp](#使用方法)
- [en](#how-to-use)

> Benefits of using ghost_installer: 
> 1. custom icons
> 2. automatic installation of ssp (if necessary)
> 3. convenient statistics for downloads
> 
> there are now 120+ releases on Taromati2's github, and the download count I'm using only counts the first 100 releases  
> With ghost_installer I can count the number of downloads directly from ghost_installer and even roll up the number of releases faster (without having to consider that the download count will drop due to new releases)  
> And I've met some new ukagaka users who don't know how to operate a computer (some of them don't even know how to unzip a file), ghost_installer also makes it easier to get started with ukagaka  
> In the past perhaps I would have been concerned about someone sharing outdated nar files that might have some major bugs in there  
> This concern was unnecessary when we switched from a guided download of nar to a guided download of ghost_installer  

## 如何使用  

下载最新的release和[ResourceHacker](http://www.angusj.com/resourcehacker/)  
使用ResourceHacker打开release中的`exe`文件，客制化你的`exe`文件  
常见修改内容：

- 图标  
  ![图片](https://user-images.githubusercontent.com/31927825/196833419-58874125-2ce2-4a52-a40c-619be3f6d183.png)  
  如果你没有合适的图标，可以在网上寻找喜欢的ico文件，或者使用ResourceHacker或者7zip从现有的exe文件中取出ico文件  
  如果你愿意，你还可以在网上找到一些png转换成ico的网站或者软件！  
  实在没有办法，你也可以使用这个[ico文件](./imgs/Installer.ico)，尽管它有点上个世纪的风格  
- NAR文件信息  
  ![图片](https://user-images.githubusercontent.com/31927825/196833448-12eb4a98-0550-4008-85ab-bd46cac4e879.png)  
  注意：保留字符串末尾的`\x00`，否则会出现错误
- 版本信息  
  ![图片](https://user-images.githubusercontent.com/31927825/196833485-6d30ede8-ade9-4116-acc8-f45ed6d5bd40.png)

## How to use  

Download the latest release and [ResourceHacker](http://www.angusj.com/resourcehacker/)  
Use ResourceHacker to open the `exe` file in the release and customise your `exe` file  
Common modifications.

- Icons  
  ![Image](https://user-images.githubusercontent.com/31927825/196833419-58874125-2ce2-4a52-a40c-619be3f6d183.png)  
  If you don't have a suitable icon, you can look for a favourite ico file online, or use ResourceHacker or 7zip to extract the ico file from an existing exe file  
  If you wish, you can also find some png to ico conversion sites or software on the internet!  
  If that's not an option, you can also use this [ico file](./imgs/Installer.ico), although it's a bit last century  
- NAR file information  
  ![image](https://user-images.githubusercontent.com/31927825/196833448-12eb4a98-0550-4008-85ab-bd46cac4e879.png)  
  Note: Keep the `\x00` at the end of the string, otherwise an error will occur
- Version information  
  ![image](https://user-images.githubusercontent.com/31927825/196833485-6d30ede8-ade9-4116-acc8-f45ed6d5bd40.png)

## 使用方法  

Releaseの最新版および [ResourceHacker](http://www.angusj.com/resourcehacker/) をダウンロードしてください。  
ResourceHackerを使って、リリースに含まれる `exe` ファイルを開き、`exe` ファイルをカスタマイズしてください。  
修正内容：

- アイコン  
  ![Image](https://user-images.githubusercontent.com/31927825/196833419-58874125-2ce2-4a52-a40c-619be3f6d183.png)  
  適当なアイコンを持っていない場合は、オンラインで好みのicoファイルを探すか、ResourceHackerや7zipを使って既存のexeファイルからicoファイルを取り出してください。  
  お望みなら、インターネット上でpngからicoへの変換サイトやソフトを探すこともできます。  
  それが難しい場合は、この[icoファイル](./imgs/Installer.ico)を使ってもよいでしょう、ちょっと前時代的ではありますが。  
- NARファイル情報  
  ![image](https://user-images.githubusercontent.com/31927825/196833448-12eb4a98-0550-4008-85ab-bd46cac4e879.png)  
  Note: `\x00` を文字列の末尾につけておかないと、エラーが発生します。
- バージョン情報  
  ![image](https://user-images.githubusercontent.com/31927825/196833485-6d30ede8-ade9-4116-acc8-f45ed6d5bd40.png)
