##
##Author: elioudakis
##A.M.:
##email: elioudakis@isc.tuc.gr
##
###################################################
#
#function   checkFile(arg1, arg2)
#The function checkFile gets two arguments, the input file (which contains correct and incorrect receipts) and the output file.
#The correct receipts are appended to the output file.
#When the user choices 2 or 3 in the main menu, the program will work with the output file.
#
###################################################



def checkFile(input_File):
    collected_receipts_file=open('tmp.txt', 'a+',encoding='utf-8')
    rec_list=input_File.read().split()
    startStopIndexes=[]
    tmp=[]

    for i in range(len(rec_list)):

        if not rec_list[i].find('-') and len(set(rec_list[i]))==1: #Check that the lines with "---" contain only this character
            startStopIndexes.append(i)

    for i in range(len(startStopIndexes)-1):
        for j in range(startStopIndexes[i]+1, startStopIndexes[i+1]): #The part of the list between two "----" 
            tmp.append(rec_list[j].upper()) 
    
        #The list tmp contains all the fields of ONE receipt.


        totalAmount=0.0

        if not(len(tmp)%4==0):
            tmp=[]
            continue

       
        if not ((tmp[0]=='ΑΦΜ:') and (len(tmp[1])==10) and tmp[1].isdigit()):
            tmp=[]
            continue

        if (tmp.count('ΑΦΜ:')!=1): ##the line for AFM exists more than once
            tmp=[]
            continue

        if (tmp.count('ΣΥΝΟΛΟ:')!=1): ##the line for total amount exists more than once
            tmp=[]
            continue
            
        if not (tmp[-2]=='ΣΥΝΟΛΟ:'):
            tmp=[]
            continue
        for k in range(2, len(tmp)-2, 4): ##the lines with the products
            #we increment the counter by 4, to access the four fields of a product each time
            #cast. Previously they were strings
            if (abs(float(tmp[k+3])-float(float(tmp[k+2])*float(tmp[k+1])))<0.0001):

                totalAmount=float(totalAmount+float(tmp[k+3]))##The product has right calculated total price. We add it to the total counter           
             
            else:##wrong total price in one of the products of the receipt
                 tmp[-1]=-1 #We set it to -1 in order to do the comparison
                 break
                        
        if not (abs(totalAmount-float(tmp[-1]))<0.0001):##wrongly calculated total amount
            tmp=[]
            continue  
       
        ##If we have reached here, the receipt is correct. We will write it to the total receipts file.

        ##
        ##The function "write" supports only 1 argument, so we will place manually the spaces and the newline
        ##
        collected_receipts_file.write('--\n')
        collected_receipts_file.write(tmp[0])
        collected_receipts_file.write(' ')
        collected_receipts_file.write(tmp[1])
        collected_receipts_file.write('\n')
        for a in range(2, len(tmp)-2, 4):
            collected_receipts_file.write(tmp[a])
            collected_receipts_file.write(' ')
            collected_receipts_file.write(tmp[a+1])
            collected_receipts_file.write(' ')
            collected_receipts_file.write(tmp[a+2])
            collected_receipts_file.write(' ')
            collected_receipts_file.write(tmp[a+3])
            collected_receipts_file.write('\n')
        collected_receipts_file.write('--\n')
            

        collected_receipts_file.flush()
        tmp=[]  ##Re-initialize the list, to have it empty in the next loop
    collected_receipts_file.close()#close the file, to update its content

def choice2():
            collected_receipts_file=open('tmp.txt',encoding='utf-8') ##we had to close and re-open the file, to update its content 
            listOfLines=[]
            collected_receipts_file.seek(0)
            for line in collected_receipts_file:
                listOfLines.append(line.split())
            data={} #defining a dictionary

            for i in listOfLines:
                if len(i)==1: #the lines with ------
                    continue #do nothing
                elif len(i)==2: #the lines with the AFM 
                    if i[0]=="ΑΦΜ:":
                        afm=i[1]
                        continue
                else: ##the lines with products
                    prodName=i[0]
                    totalPrice=i[3]

                    prodName=prodName.replace(":", "") #throw away the ':' which each prodName has
                    prodName=prodName.upper() #convert prodName to capitals

                    if prodName in data.keys():
     
                        tmp=data[prodName]

                        if afm in tmp.keys():
                            oldTotal=tmp[afm]
                            totalPrice=float(totalPrice)+float(oldTotal)
                            del tmp[afm]
                        
                        data[prodName].update({afm:float(totalPrice)})
    
                    else:
                        data.update({prodName:{afm:float(totalPrice)}})
       
            prodChoice=input('Give the product\'s name... ')
            prodChoice=prodChoice.upper()
        
            ToPrint=sorted(data[prodChoice])##list with the sorted keys of the dictionary data[prodChoice])
            for i in ToPrint:
                print(i,"%0.2f" % data[prodChoice][i])

            collected_receipts_file.close()
            
def choice3():
            collected_receipts_file=open('tmp.txt',encoding='utf-8') ##we had to close and re-open the file, to update its content 
            listOfLines=[]
            collected_receipts_file.seek(0)
            for line in collected_receipts_file:
                listOfLines.append(line.split())
            data={} #defining a dictionary

            for i in listOfLines:
                if len(i)==1: #the lines with ------
                    continue #do nothing
                elif len(i)==2: #the lines with the AFM
                    if i[0]=="ΑΦΜ:":
                        afm=i[1]
                        continue
                else: ##the lines with products
                    prodName=i[0]
                    totalPrice=i[3]

                    prodName=prodName.replace(":", "") #throw away the ':' which each prodName has
                    prodName=prodName.upper() #convert prodName to capitals

                    if afm in data.keys():
                        tmp=data[afm]
                        if prodName in tmp.keys():
                            oldTotal=tmp[prodName]
                            totalPrice=float(totalPrice)+float(oldTotal)
                            del tmp[prodName]

                        data[afm].update({prodName:float(totalPrice)})
                    
                    else:
                        data.update({afm:{prodName:float(totalPrice)}})


                    
            afmChoice=input('Give the afm... ') #afm is used as a str 
            unsortedToPrintList=data[afmChoice]

            ToPrint=sorted(data[afmChoice])##ist with the sorted keys of the dictionary data[afmChoice])
            for i in ToPrint:
                print(i,"%0.2f" % data[afmChoice][i])
     

            collected_receipts_file.close()

def main():
    
    while True:
        try:
            choice=int(input('Give your preference: (1: read new input file, 2: print statistics for a specific product, 3: print statistics for a specific AFM, 4: exit the program)'))
        except:
            continue
        if choice==1:
            input_File_Name=input('Please enter the file\'s name...')
            try:
                input_File=open(input_File_Name, 'r',encoding='utf-8')
                checkFile(input_File)
                input_File.close()
                continue
            except FileNotFoundError:
                continue
        elif choice==2:
            try:
                choice2()
                continue
            except:
                continue
        elif choice==3:
            try:
                choice3()
                continue
            except:
                continue
        elif choice==4:
            import os
            import sys
            try:
                os.remove('tmp.txt')
                sys.exit(0)
            except:
                sys.exit(0)
        else:
            continue #simply ask again for a number

if __name__ == "__main__":
    main()
