#include<iostream>
#include<opencv2/opencv.hpp>
#include<unistd.h>
#include<stdlib.h>
#include<limits.h>

using namespace std;
using namespace cv;


void printMatrix(int *matrix, int x, int y)
{
    for(int i=0;i<x;i++)
    {
        for(int j=0;j<y;j++)
        {
            cout << *(matrix+i*y+j) << " ";
        }
        cout << "\n";
    }
}

void createEnergyMatrix(Mat &img, int *energyMatrix)
{
    for(int i=0;i<img.rows;i++)
    {
        for(int j=0;j<img.cols;j++)
        {
            int x_index = i>0?i-1:img.rows-1;
            int y_index = j>0?j-1:img.cols-1;
            int x_index2 = i<img.rows-1?i+1:0;
            int y_index2 = j<img.cols-1?j+1:0;
            Vec3b pixel_up = img.at<Vec3b>(x_index, j);
            Vec3b pixel_down = img.at<Vec3b>(x_index2, j);
            Vec3b pixel_left = img.at<Vec3b>(i, y_index);
            Vec3b pixel_right = img.at<Vec3b>(i, y_index2);
            int temp = pow((pixel_up[0]-pixel_down[0]),2) + pow((pixel_up[1]-pixel_down[1]),2) + pow((pixel_up[2]-pixel_down[2]),2);
            temp+= pow((pixel_left[0]-pixel_right[0]),2) + pow((pixel_left[1]-pixel_right[1]),2) + pow((pixel_left[2]-pixel_right[2]),2);
            temp = sqrt(temp);
            *(energyMatrix+i*img.cols+j) = temp;
        }
    }
}

void createCostMatrix_width(int *energyMatrix, int rows, int cols, int *costMatrix)
{
    for(int i=0;i<rows;i++)
    {
        for(int j=0;j<cols;j++)
        {
            if(i==0)
            {
                *(costMatrix+j) = *(energyMatrix+j);
            }
            else
            {
                int temp;
                if(j==0)
                {
                    temp = min(*(costMatrix+(i-1)*cols+j), *(costMatrix+(i-1)*cols+j+1));
                }
                else if(j==cols-1)
                {
                    temp = min(*(costMatrix+(i-1)*cols+j-1), *(costMatrix+(i-1)*cols+j));
                }
                else
                {
                    temp = min(*(costMatrix+(i-1)*cols+j), min(*(costMatrix+(i-1)*cols+j-1), *(costMatrix+(i-1)*cols+j+1)));
                }
                temp += *(energyMatrix+i*cols+j);
                *(costMatrix+i*cols+j) = temp;
            }
        }
    }
}

void createCostMatrix_height(int *energyMatrix, int rows, int cols, int *costMatrix)
{
    for(int i=0;i<cols;i++)
    {
        for(int j=0;j<rows;j++)
        {
            if(i==0)
            {
                *(costMatrix+j*cols) = *(energyMatrix+j*cols);
            }
            else
            {
                int temp;
                if(j==0)
                {
                    temp = min(*(costMatrix+i-1), *(costMatrix+cols+i-1));
                }
                else if(j==rows-1)
                {
                    temp = min(*(costMatrix+(rows-1)*cols+i-1), *(costMatrix+(rows-2)*cols+i-1));
                }
                else
                {
                    temp = min(*(costMatrix+(j-1)*cols+i-1), min(*(costMatrix+j*cols+i-1), *(costMatrix+(j+1)*cols+i-1)));
                }
                //cout << temp;
                temp += *(energyMatrix+j*cols+i);
                *(costMatrix+j*cols+i) = temp;
            }
        }
    }
}

void getPath_width(int* costMatrix, int rows, int cols, int *path)
{
    int path_index=0;
    int cost_index=-1;
    int minimum = INT_MAX;
    for(int j=0;j<cols;j++)
    {
        if(*(costMatrix+(rows-1)*cols+j)<minimum)
        {
            minimum = *(costMatrix+(rows-1)*cols+j);
            cost_index = j;
        }
    }
    path[path_index] = cost_index;
    path_index++;
    for(int i=rows-2;i>=0;i--)
    {
        if(cost_index==0)
        {
            int val1 = *(costMatrix+(i-1)*cols);
            int val2 = *(costMatrix+(i-1)*cols+1);
            path[path_index] = val1<val2?cost_index:++cost_index;
        }
        else if(cost_index==cols-1)
        {
            int val1 = *(costMatrix+(i-1)*cols+cost_index-1);
            int val2 = *(costMatrix+(i-1)*cols+cost_index);
            path[path_index] = val1<val2?--cost_index:cost_index;
        }
        else
        {
            int val1 = *(costMatrix+(i-1)*cols+cost_index-1);
            int val2 = *(costMatrix+(i-1)*cols+cost_index);
            int val3 = *(costMatrix+(i-1)*cols+cost_index+1);
            path[path_index] = (val1<val2 && val1<val3)?--cost_index:(val2<val3)?cost_index:++cost_index;
        }
        path_index++;
    }
}

void getPath_height(int* costMatrix, int rows, int cols, int *path)
{
    int path_index=0;
    int cost_index=-1;
    int minimum = INT_MAX;
    for(int j=0;j<rows;j++)
    {
        if(*(costMatrix+cols*j)<minimum)
        {
            minimum = *(costMatrix+cols*j);
            cost_index = j;
        }
    }
    path[path_index] = cost_index;
    //cout << cost_index << "\n";
    path_index++;
    for(int i=cols-2;i>=0;i--)
    {
        if(cost_index==0)
        {
            int val1 = *(costMatrix+i-1);
            int val2 = *(costMatrix+cols+i-1);
            path[path_index] = val1<val2?cost_index:++cost_index;
        }
        else if(cost_index==rows-1)
        {
            int val1 = *(costMatrix+(rows-1)*cols+i-1);
            int val2 = *(costMatrix+(rows-2)*cols+i-1);
            path[path_index] = val1<val2?--cost_index:cost_index;
        }
        else
        {
            int val1 = *(costMatrix+(cost_index-2)*cols+i-1);
            int val2 = *(costMatrix+(cost_index-1)*cols+i-1);
            int val3 = *(costMatrix+cost_index*cols+i-1);
            path[path_index] = (val1<val2 && val1<val3)?--cost_index:(val2<val3)?cost_index:++cost_index;
        }
        path_index++;
    }
}

void setPathRed(Mat &img, int *path, int rows, int cols, int mode)
{
    Vec3b pixel_red;
    pixel_red[0] = 0;
    pixel_red[1] = 0;
    pixel_red[2] = 255;
    if(mode==0)
    {
        for(int i=0;i<rows;i++)
        {
            img.at<Vec3b>(i, path[rows-i-1]) = pixel_red;
        }
    }
    else
    {
        for(int i=0;i<cols;i++)
        {
            img.at<Vec3b>(path[cols-i-1], i) = pixel_red;
        }
    }
}

void removePath(Mat &img, int *path, int rows, int cols, int mode)
{
    if(mode==0)
    {
        for(int i=0;i<rows;i++)
        {
            for(int j=path[rows-i-1];j<cols-1;j++)
            {
                img.at<Vec3b>(i, j) = img.at<Vec3b>(i, j+1);
            }
        }
    }
    else
    {
        for(int i=0;i<cols;i++)
        {
            for(int j=path[cols-i-1];j<rows-1;j++)
            {
                img.at<Vec3b>(j, i) = img.at<Vec3b>(j+1, i);
            }
        }
    }
}

void seamCarving(string filename, int req_rows, int req_cols)
{
    Mat img = imread(filename, IMREAD_COLOR);
    int rows = img.rows;
    int cols = img.cols;
    if(req_rows> rows || req_cols > cols)
    {
        cout << "Invalid size input\n";
        exit(1);
    }
    for(int i=0;i<cols-req_cols;i++)
    {
        int costMatrix[rows][cols-i] = {-1};
        int energyMatrix[rows][cols-i] = {0};
        int path[rows] = {-1};
        createEnergyMatrix(img, (int*)energyMatrix);
        createCostMatrix_width((int*)energyMatrix, rows, cols-i, (int*)costMatrix);
        getPath_width((int*)costMatrix, rows, cols-i, path);
        setPathRed(img, path, rows, cols-i, 0);
        imshow("Image", img);
        waitKey(200);
        removePath(img, path, rows, cols-i, 0);
        img = img(Range::all(), Range(0,cols-i-1));
        imshow("Image", img);
        waitKey(50);
    }
    cols = req_cols;
    for(int i=0;i<rows-req_rows;i++)
    {
        int costMatrix[rows-i][cols] = {-1};
        int energyMatrix[rows-i][cols] = {0};
        int path[cols] = {-1};
        createEnergyMatrix(img, (int*)energyMatrix);
        //printMatrix((int*)energyMatrix, rows, cols);
        createCostMatrix_height((int*)energyMatrix, rows-i, cols, (int*)costMatrix);
        //printMatrix((int*)costMatrix, rows, cols);
        getPath_height((int*)costMatrix, rows-i, cols, path);
        // for(int i=0;i<cols;i++)
        // {
        //     cout << path[i] << " ";
        // }
        // exit(1);
        setPathRed(img, path, rows-i, cols, 1);
        imshow("Image", img);
        waitKey(200);
        removePath(img, path, rows-i, cols, 1);
        img = img(Range(0,rows-i-1), Range::all());
        imshow("Image", img);
        waitKey(50);
    }
    destroyWindow("Image");
    string opfilename = filename.substr(0,filename.find_last_of('.'));
    opfilename+="_reduced.jpg";
    imwrite(opfilename, img);

    //printMatrix((int*)costMatrix, rows, cols);
    // for(int i=0;i<rows;i++)
    // {
    //     cout << path[i] << " ";
    // }
    //cout << findSeam(energyMatrix, seamIndices, 0, 0, 0, 0);

    // for(int i=0;i<img.rows-len;i++)
    // {
    //     vector<vector<int>> seamIndices(img.rows-i, vector<int>(img.cols));
    //     vector<int> seamCosts(img.cols);
    //     for(int j=0;j<img.rows-i;j++)
    //     {
    //         seamIndices[j][0] = j;
    //         seamCosts[j] = findSeam(energyMatrix, seamIndices, j, j, 0, 0);
    //     }
    // }

    //printMatrix(ret);

    //cv::imshow("Image", img);
    //waitKey(2000);
    
}

int main(int argc, char** argv)
{
    string name, path;
    char buffer[100];
    int h, w;
    cout << "Enter the file name\n";
    getline(cin, name);
    // getcwd(buffer, 100);
    // path = buffer;
    // path = path + "/" + name;
    // cout << path << "\n";
    cout << "Enter the required height and width required separated by space\n";
    cin >> h >> w;
    seamCarving(name, h, w);
    return 0;
}