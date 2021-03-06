#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <cmath>
#include <linux/can.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include "SDL2_gfxPrimitives.h"


#define Widget_BUTTON 1
#define Widget_Label  2
#define Widget_VGRAPH 3
#define Widget_BITMAP 10
#define Widget_PagePicker 11
#define Widget_TextView 12
#define Widget_Table 13
#define Widget_Grapher 14

#define Window_Width 850
#define Window_Height 850

#define DASHBOARD_WIDGET 0

/* Notes
 * Choose auto x when you want to work with copies.
   Choose auto &x when you want to work with original items and may modify them.
   Choose auto const &x when you want to work with original items and will not modify them.
 */

//Forward Declarations
class Renderer;
class DashboardWidget;
class menuPage;

void DrawRectangle_FIX(SDL_Renderer* renderer, SDL_Rect* rect)
{
  uint8_t r,g,b,a;
  SDL_GetRenderDrawColor(renderer,&r,&g,&b,&a);
  int lineWidth = 2;
  thickLineRGBA(renderer,rect->x,rect->y,rect->x+rect->w,rect->y,lineWidth,r,g,b,a);
  thickLineRGBA(renderer,rect->x,rect->y,rect->x,rect->y+rect->h,lineWidth,r,g,b,a);
  thickLineRGBA(renderer,rect->x,rect->y+rect->h,rect->x+rect->w,rect->y+rect->h,lineWidth,r,g,b,a);
  thickLineRGBA(renderer,rect->x+rect->w,rect->y+rect->h,rect->x+rect->w,rect->y,lineWidth,r,g,b,a);
}

struct Context{
    Renderer* RENDERER;//Renderer being used to draw to the screen
    SDL_Renderer* renderer;
    menuPage* currentPage;//Page to which the widget belongs.
};

struct pagePickerData{
    int selectedIconIndex = 0;
};

void loadPage(Renderer* renderer,menuPage* page);
struct Context* getContext(Renderer* RENDERER);

class DashboardWidget{
public:
    int xpos;
    int ypos;
    int width;
    int height;
    char widgetName[10]{};
    int widgetType = -1;
    bool isSelected = false;
    char auxillaryData[250]{};// Storage for widgets that need extra information; such as the pagepicker for icon selection
    DashboardWidget(int XPOS, int YPOS, int WIDTH, int HEIGHT, char WIDGET_NAME[10], int WIDGET_TYPE)
    {
        xpos = XPOS;
        ypos = YPOS;
        width = WIDTH;
        height = HEIGHT;
        strcpy(widgetName, WIDGET_NAME);
        widgetType = WIDGET_TYPE;
    }
    virtual void onDraw(Renderer *RENDERER) = 0;
    virtual void onClick(Renderer* RENDERER) = 0;
};

class menuPage
{
protected:

public:
    struct widget_array_entry{
        int widgetType;
        void* widgetPTR;
    };
    std::vector<widget_array_entry> widgetVector;
    char title[20]{};
    int selectedItem = 0;
  void incrementSelectedWidget();
    void decrementSelectedWidget();
    void addWidget(DashboardWidget* inputWidget);
    explicit menuPage(SDL_Window* pwindow)
    {

    }

    void setTitle(char* newTitle);
};

void menuPage::addWidget(DashboardWidget *inputWidget) {
    widget_array_entry entry{};
    entry.widgetType = DASHBOARD_WIDGET;
    entry.widgetPTR = (void*)inputWidget;
    widgetVector.push_back(entry);
}

void menuPage::setTitle(char *newTitle) {
    strcpy(title,newTitle);
}

void menuPage::incrementSelectedWidget() {
    //If the page has a pagePicker direct these "increment requests" into the pagepicker widget so icons are selected accordingly
    auto* widget = (DashboardWidget*)widgetVector[0].widgetPTR;
    if(widgetVector.size() == 1 && widget->widgetType == Widget_PagePicker)
    {
        DashboardWidget* pagePicker = widget;
        auto* pickerData = (struct pagePickerData*)(pagePicker->auxillaryData);
        pickerData->selectedIconIndex++;
    } else
    {
        selectedItem++;
    }
}

void menuPage::decrementSelectedWidget() {
    //If the page has a pagePicker direct these "decrement" requests" into the pagepicker widget so icons are selected accordingly
    auto* widget = (DashboardWidget*)widgetVector[0].widgetPTR;
    if(widgetVector.size() == 1 && widget->widgetType == Widget_PagePicker)
    {
        DashboardWidget* pagePicker = widget;
        auto* pickerData = (struct pagePickerData*)(pagePicker->auxillaryData);
        if(pickerData->selectedIconIndex == 0)
        {

        } else
        {
            pickerData->selectedIconIndex--;
        }
    }
    else
    {
        selectedItem--;
    }
}

//////////////////////End menuPage functions//////////////////////

//Widgets//
class VerticalGraph : public DashboardWidget{
public:
    float value = 0.0f;
    float maxValue = 0.0f;// Used to determine the scale of the graph
    char unitName[10]{};
    int tickDivision;
    TTF_Font* Sans = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 45);
    TTF_Font* SansOilGaugeTicks = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 45);
    VerticalGraph(int XPOS, int YPOS, float MAX_Value, int tickDIVISION, char* graphName, char* UNITNAME): DashboardWidget(XPOS, YPOS, 125, 260, graphName, Widget_VGRAPH)
    {
        maxValue = MAX_Value;
        tickDivision = tickDIVISION;
        strcpy(unitName,UNITNAME);

    }

    void setValue(float newValue)
    {
        value = newValue;
    }

    void onClick(Renderer* RENDERER) override
    {

    }

    void onDraw(Renderer *RENDERER) override
    {
        Context* context = getContext(RENDERER);
        //Draw outer Rectangle
        SDL_Rect rect;
        rect.x = xpos;
        rect.y = ypos;
        rect.w = width;
        rect.h = height;
        SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        //DrawRectangle_FIX(context->renderer,&rect);
        rect.x = xpos-1;
        rect.y = ypos-1;
        rect.w = width+1;
        rect.h = height+1;
        //DrawRectangle_FIX(context->renderer,&rect);
      //Draw Graph Name
      SDL_Color textColor = {255, 255, 0};
      SDL_Surface *surfaceMessage;
      surfaceMessage = TTF_RenderText_Solid(Sans,
                                            widgetName,
                                            textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
      SDL_Texture *Message;
      Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
      SDL_Rect Message_rect; //create a rect
      Message_rect.x = xpos+(width/2)-25;
      Message_rect.y = ypos;
      Message_rect.w = 50; // controls the width of the rect
      Message_rect.h = 40; // controls the height of the rect
      SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
      SDL_DestroyTexture(Message);
      SDL_FreeSurface(surfaceMessage);
      //Draw inner rectangle for bar value
      rect.x = (xpos + (width/2)) - (25/2);
      rect.y = ypos + 50;
      rect.w = 25;
      rect.h = 150;
      DrawRectangle_FIX(context->renderer, &rect);
      //Draw Bar (Vertical)
      SDL_SetRenderDrawColor(context->renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
      rect.x = ((xpos + (width/2)) - (25/2))+2;
      rect.y = ypos + 52;
      rect.w = 21;
      rect.h = (150*value)/(maxValue); // NOLINT(cppcoreguidelines-narrowing-conversions,bugprone-narrowing-conversions)
      SDL_RenderFillRect(context->renderer, &rect);
      //Draw Tick Numbers
      textColor = {0, 255, 0};
      for(int index = 0; index < 4; index++)
      {
        std::string tickValue = std::to_string(index*tickDivision);
        rect.x = ((xpos + (width/2)) - (25/2))-40;
        rect.y = ypos + 50 + (150*index*tickDivision)/(maxValue)-(30/2);
        rect.w = 25;
        rect.h = 30;
        surfaceMessage = TTF_RenderText_Solid(SansOilGaugeTicks, tickValue.c_str(), textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
        Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
        SDL_RenderCopy(context->renderer, Message, nullptr, &rect);
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);
      }
      //Draw Value text
      textColor = {255,255,255};
      std::string valueText = std::to_string((int)value) + " " +std::string(unitName);
      rect.x = xpos+(50/2)-10;
      rect.y = ypos+height-45;
      rect.w = 100;
      rect.h = 40;
      surfaceMessage = TTF_RenderText_Solid(SansOilGaugeTicks, valueText.c_str(), textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
      Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
      SDL_RenderCopy(context->renderer, Message, nullptr, &rect);
      SDL_DestroyTexture(Message);
      SDL_FreeSurface(surfaceMessage);
    }

};

class label : public DashboardWidget
{
public:
    char labelValue[20]{};
    int labelColor = 0;
    menuPage* onClick_NextPage = nullptr;
    TTF_Font* Sans = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 45);
    label(char labelName[], int x, int y, int Color) : DashboardWidget(x, y, 250, 45, labelName, Widget_Label)
    {
        strcpy(labelValue,labelName);
        widgetType = Widget_Label;
        labelColor = Color;
    }

  void onClick(Renderer* RENDERER) override
    {

    }

};

class Grapher : public DashboardWidget{

 private:
  struct coordinate{
    float x;
    float y;
  };
  int yAxisTickFontSize = 0;
  int xAxisTickFontSize = 0;
  std::vector<struct coordinate> graphPoints;

 public:
  int xmin,ymin,xmax,ymax;
  int xPrime = xpos+75;//X position of coordinate plane
  int yPrime = ypos+(height-75);//Y position of coordinate plane
  int coordinatePlaneXOffset = 75;
  int coordinatePlaneYOffset = 75;
  TTF_Font *Font_Sizes[200]{};//from 1-49 point size
  Grapher(int XPOS, int YPOS, int Width, int Height) : DashboardWidget(XPOS,YPOS,Width,Height,(char*)"Grapher",Widget_Grapher)
  {
    //initialize Fonts
    for(int i = 0; i <200; i++)
    {
      Font_Sizes[i] = TTF_OpenFont("/home/dylan/Desktop/sans/Orbitron-Black.ttf", i+1);
    }
  }

  void graphPoint(float x, float y)
  {
    struct coordinate c;
    c.x = x;
    c.y = y;
    graphPoints.push_back(c);

  }

  void setYAxisTickFontSize(int fontsize)
  {
    this->yAxisTickFontSize = fontsize;
  }

  void setXAxisTickFontSize(int fontsize)
  {
    this->xAxisTickFontSize = fontsize;
  }

  void onClick(Renderer* RENDERER) override
  {
    int i = 0;
  }

  TTF_Font* getFont(int pointSize)
  {
    return Font_Sizes[pointSize+1];
  }

  int drawPointCoordinatePlane(SDL_Renderer* renderer, float x, float y)
  {
    if(x > xmax || x < xmin || y > ymax || y < ymin)
    {
      return -1;
    }
    //Compute the offset from the prime coordinates.
    float scaledXCoordinate = ((x*((xpos+width)-xPrime))/(xmax-xmin))-((xmin*((xpos+width)-xPrime))/(xmax-xmin))+xPrime;//actual screen x coordinate on coordinate plane
    float scaledYCoordinate = ((y*(ypos-yPrime))/(ymax-ymin))-((ymin*(ypos-yPrime))/(ymax-ymin))+yPrime;
    //SDL_RenderDrawLine(renderer,0,0,scaledXCoordinate,scaledYCoordinate);
    SDL_RenderDrawPointF(renderer,scaledXCoordinate,scaledYCoordinate);
    SDL_RenderDrawPointF(renderer,scaledXCoordinate+1,scaledYCoordinate+1);
    SDL_RenderDrawPointF(renderer,scaledXCoordinate-1,scaledYCoordinate+1);

  }

  void setScale(int XMIN, int YMIN, int XMAX, int YMAX)
  {
    this->xmin = XMIN;
    this->ymin = YMIN;
    this->xmax = XMAX;
    this->ymax = YMAX;
  }

  void clearPoints()
  {
    graphPoints.clear();
  }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-narrowing-conversions"
  void onDraw(Renderer *RENDERER) override
  {
    Context* context = getContext(RENDERER);
    SDL_Rect rect;
    rect.x = xpos-1;
    rect.y = ypos-1;
    rect.w = width-1;
    rect.h = height-1;
    SDL_SetRenderDrawColor(context->renderer,255,255,255,255);
    //DrawRectangle_FIX(context->renderer,&rect);
    SDL_SetRenderDrawColor(context->renderer,0,255,0,255);
    //Draw coordinate plane(horizontal)
    int y = ypos;
    int ySpacing = (height-75)/8;
    int count = 0;
    while(count < 9)
    {
      for(int x = xpos + 75; x < xpos + width; x++)
      {
        SDL_RenderDrawPoint(context->renderer, x,y);
        SDL_RenderDrawPoint(context->renderer, x,y-1);
      }
      y = y + ySpacing;
      count++;
    }
    //Draw coordinate plane(vertical)
    int x = xpos + 75;
    int xSpacing = (width - 75)/8;
    count = 0;
    while(count < 9)
    {
      for(int y = ypos; y < ypos + height - 75; y++)
      {
        SDL_RenderDrawPoint(context->renderer, x,y);
        SDL_RenderDrawPoint(context->renderer, x+1,y);
      }
      x = x + xSpacing;
      count++;
    }
    //Draw tick labels
    SDL_Color textColor = {255,255,0};
    int xIndex = 0;
    count = 0;
    while(xIndex < xmax+25)
    {
      std::string value = std::to_string(count);
      SDL_Surface *surfaceMessage;
      SDL_Rect Message_rect;
      float scaledXCoordinate = ((xIndex*((xpos+width)-xPrime))/(xmax-xmin))-((xmin*((xpos+width)-xPrime))/(xmax-xmin))+xPrime;
      if(this->xAxisTickFontSize != 0)
      {
        surfaceMessage = TTF_RenderText_Solid(getFont(this->xAxisTickFontSize),
                                              value.c_str(),
                                              textColor);
        Message_rect.x = scaledXCoordinate-(xSpacing*.75)/2;
        Message_rect.y = yPrime;
        Message_rect.w = this->xAxisTickFontSize*.75; // controls the width of the rect
        Message_rect.h = this->xAxisTickFontSize*.75; // controls the height of the rect
      } else
      {
        surfaceMessage = TTF_RenderText_Solid(getFont(xSpacing),
                                              value.c_str(),
                                              textColor);
        Message_rect.x = scaledXCoordinate-(xSpacing*.75)/2;
        Message_rect.y = yPrime;
        Message_rect.w = xSpacing*.75; // controls the width of the rect
        Message_rect.h = xSpacing*.75; // controls the height of the rect
      }

      SDL_Texture *Message;
      Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
      SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
      SDL_DestroyTexture(Message);
      SDL_FreeSurface(surfaceMessage);
      xIndex = xIndex + 25;
      count = count + 25;
    }

    int yIndex = 25;
    count = 25 ;
    SDL_Rect Message_rect;
    while(yIndex < ymax+25)
    {
      std::string value = std::to_string(count);
      SDL_Surface *surfaceMessage;
      float scaledYCoordinate = ((yIndex*(ypos-yPrime))/(ymax-ymin))-((ymin*(ypos-yPrime))/(ymax-ymin))+yPrime;
      if(this->yAxisTickFontSize != 0)
      {
        surfaceMessage = TTF_RenderText_Solid(getFont(this->yAxisTickFontSize),
                                              value.c_str(),
                                              textColor);
        Message_rect.x = xPrime - 50;
        Message_rect.y = scaledYCoordinate-((ySpacing*.75)/2);
        Message_rect.h = this->yAxisTickFontSize*.75;
        Message_rect.w = this->yAxisTickFontSize*.75;
      }
      else
      {
        surfaceMessage = TTF_RenderText_Solid(getFont(xSpacing),
                                              value.c_str(),
                                              textColor);
        Message_rect.x = xPrime - 50;
        Message_rect.y = scaledYCoordinate-((ySpacing*.75)/2);
        Message_rect.h = ySpacing*.75;
        Message_rect.w = ySpacing*.75;
      }
      SDL_Texture *Message;
      Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);

      SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
      SDL_DestroyTexture(Message);
      SDL_FreeSurface(surfaceMessage);
      yIndex = yIndex + 25;
      count = count + 25;
    }

    textColor = {0,255,0};
    SDL_Surface *surfaceMessage;
    surfaceMessage = TTF_RenderText_Solid(getFont(20),
                                          "PSI",
                                          textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
    SDL_Texture *Message;
    Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
    yIndex = 50;
    float scaledYCoordinate = ((yIndex*(ypos-yPrime))/(ymax-ymin))-((ymin*(ypos-yPrime))/(ymax-ymin))+yPrime;
    Message_rect.x = xPrime-95;
    Message_rect.y = scaledYCoordinate-(35/2)-2;
    Message_rect.w = 35; // controls the width of the rect
    Message_rect.h = 35; // controls the height of the rect
    SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
    SDL_DestroyTexture(Message);
    //SDL_FreeSurface(surfaceMessage);
    xIndex = 50;
    Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
    float scaledXCoordinate = ((xIndex*((xpos+width)-xPrime))/(xmax-xmin))-((xmin*((xpos+width)-xPrime))/(xmax-xmin))+xPrime;
    Message_rect.x = scaledXCoordinate-(35/2)-2;
    Message_rect.y = yPrime+30;
    Message_rect.w = 35; // controls the width of the rect
    Message_rect.h = 35; // controls the height of the rect
    SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);
    SDL_SetRenderDrawColor(context->renderer,255,0,0,255);
    //Draw user points.
    for(auto& coordinate : graphPoints)
    {
      drawPointCoordinatePlane(context->renderer,coordinate.x,coordinate.y);
    }
  }
#pragma clang diagnostic pop

};

class BitmapWidget : public DashboardWidget{
private:
    char filePath[250]{};
    int startIndex = 0;
    int endIndex = 0;
    int value = 0;
    float slope = 0;
    float B = 0;
public:
    BitmapWidget(int XPOS,
                 int YPOS,
                 int Width,
                 int Height,
                 char *FILEPATH,
                 int START_INDEX,
                 int END_INDEX) : DashboardWidget(XPOS, YPOS, Width, Height, (char*)"BITMAPW", Widget_BITMAP)
    {
        strcpy(filePath,FILEPATH);
        startIndex = START_INDEX;
        endIndex = END_INDEX;
    }
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "bugprone-narrowing-conversions"
    /*Provide initial conditions to compute correct bitmap indexing (linear regression method)
     * For example, provide a gauge value as input, and then provide its corresponding index#
     * if the gauge says, 55, its corresponding bitmap is at index 1005; or something like that.
     */
    void setInitalConditions(float input1, float actualGaugeIndex1, float input2, float actualGaugeIndex2)
    {
      slope = (actualGaugeIndex2-actualGaugeIndex1)/(input2-input1);
      B = (actualGaugeIndex2 - slope*input2);
    }
#pragma clang diagnostic pop

    void setValue(float newValue)
    {
        if(slope != 0)
        {
          value = (int)(slope*newValue+B);
        }
    }

    void onClick(Renderer* RENDERER) override
    {
        int i = 0;
    }

    void onDraw(Renderer *RENDERER) override
    {
        Context* context = getContext(RENDERER);
        if(value > endIndex)
        {
            value = endIndex;
        } else {

        }
        std::string Path = std::string(this->filePath) + std::to_string(value) + ".bmp";
        SDL_Surface *image = SDL_LoadBMP(Path.c_str());
        SDL_Texture *texture = SDL_CreateTextureFromSurface(context->renderer, image);
        SDL_Rect r;
        r.x = xpos;
        r.y = ypos;
        r.w = width;
        r.h = height;
        SDL_RenderCopy(context->renderer, texture, nullptr, &r);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(image);
        ////////return addr.can_addr;////////////////////
        //DrawRectangle_FIX(renderer,&r);
    }

};

class pagePicker : public DashboardWidget
{
public:
    TTF_Font* Sans = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 50);

    struct iconStruct{
        char* iconPath;//File path for icon
        char iconName[50];//Name to be displayed under icon
        menuPage* onClickPagePointer;//Page to load when button is clicked.
    };
    std::vector<struct iconStruct> iconVector;

    pagePicker(int x, int y, int width, int height) : DashboardWidget(x, y, width, height, "Drawer", Widget_PagePicker)
    {
        struct pagePickerData pickerData;
        pickerData.selectedIconIndex = 0;
        memcpy(auxillaryData,&pickerData,sizeof(struct pagePickerData));
    }


    void addItem(struct iconStruct inputSTRUCT)
    {
        iconVector.push_back(inputSTRUCT);
    }

    void onClick(Renderer* RENDERER) override//When an icon is selected, go to that page
    {
        //Get the current icon
        auto* pickerData = (struct pagePickerData*)(&auxillaryData[0]);
        iconStruct currentIcon = iconVector[pickerData->selectedIconIndex];
        loadPage(RENDERER,currentIcon.onClickPagePointer);
        //Somehow get the renderer to change current page
        //RENDERER->loadPage(currentIcon.onClickPagePointer);
    }

    void onDraw(Renderer *RENDERER) override
    {
        auto* context = (Context*)getContext(RENDERER);
        SDL_Rect rect;
        //Draw upper menu graphic
        rect.x = xpos;
        rect.y = ypos;
        rect.w = width;
        rect.h = 100;
        SDL_Surface *image = SDL_LoadBMP("/home/dylan/Desktop/Comp 10000.bmp");//Load Image, upper menu graphic
        SDL_Texture *texture = SDL_CreateTextureFromSurface(context->renderer, image);
        SDL_RenderCopy(context->renderer, texture, nullptr, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(image);
        //Draw outer Rectangle
        rect.x = xpos;
        rect.y = ypos;
        rect.w = width;
        rect.h = height;
        SDL_SetRenderDrawColor(context->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        DrawRectangle_FIX(context->renderer, &rect);//Draw the outer rectangle of the console window
        /*
        //Draw the 2x3 grid
        for(int i = 0; i < 3; i++)//Vertical Lines
        {
            SDL_RenderDrawLine(renderer,i*200,rect.y+100,i*200,rect.y+height);
        }
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        for(int i = 0; i < 3; i++)//Horizontal Lines
        {
            SDL_RenderDrawLine(renderer,0,i*200+rect.y+100,rect.x+width,i*200+rect.y+100);
        }
         */
        //Index the iconStruct vector
        int rowCount = 0;
        int colCount = 0;
        for(int index = 0; index < iconVector.size(); index++)
        {
            if(index % 3 == 0 && index != 0)//Logic(s) for drawing the icons on the grid.
            {
                rowCount++;
            }
            colCount = index % 3;
            struct iconStruct tempStruct = iconVector[index];
            //Draw the icons onto the grid
            image = SDL_LoadBMP(tempStruct.iconPath);//Load Image
            texture = SDL_CreateTextureFromSurface(context->renderer, image);
            //Determine the rectangle for each grid square
            SDL_Rect gridRectangle;
            gridRectangle.x = colCount*200+50;
            gridRectangle.y = rowCount*200+rect.y+150;
            gridRectangle.w = 100;
            gridRectangle.h = 100;
            SDL_RenderCopy(context->renderer, texture, nullptr, &gridRectangle);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(image);
            //*******Draw Text**********
            SDL_Color textColor = {255, 255, 0};
            auto* pickerData = (struct pagePickerData*)(&auxillaryData[0]);
            if(index == pickerData->selectedIconIndex)
            {
                textColor = {255, 0, 0};
            }

            SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, tempStruct.iconName, textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
            SDL_Texture* Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
            SDL_Rect Message_rect; //create a rect
            Message_rect.x = gridRectangle.x;
            Message_rect.y = gridRectangle.y + gridRectangle.h + 5;
            Message_rect.w = gridRectangle.w; // controls the width of the rect
            Message_rect.h = 40; // controls the height of the rect
            SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
            SDL_DestroyTexture(Message);
            SDL_FreeSurface(surfaceMessage);
        }
        //End of for loop

    }
};

class TextView : public DashboardWidget {
public:
    TTF_Font *Sans = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 40);
    int stringCount = 0;
    std::vector<std::string> textStrings;
    TTF_Font *Font_Sizes[50]{};//from 1-49 point size
    TextView(int x, int y, int width, int height) : DashboardWidget(x, y, width, height, (char*)"Button", Widget_TextView)
    {
        //initilize stringVector
        for(int i = 0; i < 100; i++)
        {
            textStrings.push_back("");
        }
        //initialize Fonts
        for(int i = 0; i <50; i++)
        {
            Font_Sizes[i] = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", i+1);
        }
    }

    TTF_Font* getFont(int pointSize)
    {
        return Font_Sizes[pointSize+1];
    }

    void insertString(std::string str)//Add String to textview. Text will loop over.
    {
        textStrings[stringCount] = str;
        stringCount++;
        stringCount = stringCount % (height/40);
    }

    void onDraw(Renderer *RENDERER) override {
    Context* context = getContext(RENDERER);
    //Create the background rectangle (white)
    SDL_Rect widgetRect;
    widgetRect.x = xpos;
    widgetRect.y = ypos;
    widgetRect.w = width;
    widgetRect.h = height;
    SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    DrawRectangle_FIX(context->renderer,&widgetRect);
        SDL_SetRenderDrawColor(context->renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
    //Draw grid (debugging)
    SDL_Rect textRect;
    SDL_Color textColor = {255,255,255};
    int y = ypos;
    int size = 40;
    Sans = getFont(size);
    for(auto & textString : textStrings)
    {
        int w,h;
        TTF_SizeText(Sans,textString.c_str(),&w,&h);
        //if the text width exceeds the width of the textview scale down.
        if(w > width)
        {
            std::string str = textString;
            while(w > width)
            {
                Sans = getFont(size);
                TTF_SizeText(Sans,textString.c_str(),&w,&h);
                size--;
            }
        }
        textRect.x = xpos;
        textRect.y = y;
        textRect.w = w;
        textRect.h = size+7;
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, textString.c_str(), textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
        SDL_Texture* Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
        SDL_RenderCopy(context->renderer, Message, nullptr, &textRect);
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);
        y = y + size + 5;
    }

    }

    void onClick(Renderer* RENDERER) override//When button is selected, run the click handler
    {

    }
};
/*
 * Simple table for displaying information
 */
class Table : public DashboardWidget{
public:
    struct item{
        char name[10];
        char value[10];
    };
    TTF_Font *Font_Sizes[50]{};//from 1-49 point size
    std::vector<struct item> items;

    Table(int x, int y,int width, int height) : DashboardWidget(x,y,width,height,"Table",Widget_Table)
    {
        //initialize Fonts
        for(int i = 0; i <50; i++)
        {
            Font_Sizes[i] = TTF_OpenFont("/home/dylan/Desktop/sans/Orbitron-Black.ttf", i+1);
        }
        //initialize items
        struct item emptyStruct{};
        strcpy(emptyStruct.name,"");
        for(int i = 0; i < 5; i++)
        {
            items.push_back(emptyStruct);
        }
    }

    void setValue(int index, char* name, char* value)
    {
        strcpy(items[index].name,name);
        strcpy(items[index].value,value);

    }

    TTF_Font* getFont(int pointSize)
    {
        return Font_Sizes[pointSize+1];
    }

    void onDraw(Renderer *RENDERER) override {
        Context *context = getContext(RENDERER);
        SDL_Rect rect;
        rect.x = xpos;
        rect.y = ypos;
        rect.w = width;
        rect.h = height;
        SDL_SetRenderDrawColor(context->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        //DrawRectangle_FIX(context->renderer,&rect);
        //Draw values
        int y = ypos;
        int index = 0;
        int w,h;
        while(y < ypos+height)
        {
            SDL_Color textColor = {255,255,0};
            SDL_Surface* surfaceMessage = TTF_RenderText_Solid(getFont(20), items[index].name, textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
            SDL_Texture* Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
            SDL_Rect Message_rect; //create a rect
            Message_rect.x = xpos+(width*.05);
            Message_rect.y = y;
            Message_rect.w = width*.40;
            Message_rect.h = 45;
            SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
            SDL_DestroyTexture(Message);
            SDL_FreeSurface(surfaceMessage);

            textColor = {255,255,0};
            surfaceMessage = TTF_RenderText_Solid(getFont(20), items[index].value, textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
            Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
            Message_rect.x = xpos+width*.55;
            Message_rect.y = y;
            TTF_SizeText(getFont(20),items[index].value,&w,&h);
            Message_rect.w = w;//width*.40;
            Message_rect.h = 45;
            SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
            SDL_DestroyTexture(Message);
            SDL_FreeSurface(surfaceMessage);

            y = y + 50;
            index++;
        }


        //SDL_RenderDrawLine(context->renderer,xpos+(width/2),ypos,xpos+(width/2),ypos+height);

    }

    void onClick(Renderer* RENDERER) override//When button is selected, run the click handler
    {

    }

};

class Button : public DashboardWidget{
public:
    TTF_Font* Sans = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 75);
    void (*onClick_functionPTR)() = nullptr;
    Button(int x, int y, char* buttonName) : DashboardWidget(x,y,width,height,(char*)"Button",Widget_BUTTON)
    {
        strcpy(this->widgetName,buttonName);
        width = 200;
        height = 45;
    }

    void onDraw(Renderer *RENDERER) override
    {
        Context* context = getContext(RENDERER);
        int count = 0;
        for(int i = 0; i < height; i++)//Draw the background for the button
        {
            SDL_RenderDrawLine(context->renderer, xpos, ypos + i, xpos + width, ypos + i);
            SDL_SetRenderDrawColor(context->renderer, count, count, count, SDL_ALPHA_OPAQUE);
            count+= 5;
        }
        //Draw Button Rectangle
        SDL_Rect outline;
        outline.x = xpos;
        outline.y = ypos;
        outline.h = height;
        outline.w = width;
        SDL_SetRenderDrawColor(context->renderer, 51, 51, 255, SDL_ALPHA_OPAQUE);
        DrawRectangle_FIX(context->renderer, &outline);
        //Draw the Button name
        //Determine if button is selected
        SDL_Color textColor;
        if(this->isSelected)
        {
            textColor = {0, 255, 0};
        } else
        {
            textColor = {255, 255, 255};
        }

        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, this->widgetName, textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
        SDL_Texture* Message = SDL_CreateTextureFromSurface(context->renderer, surfaceMessage);
        SDL_Rect Message_rect; //create a rect
        Message_rect.x = xpos+(width/2)-(100/2);
        Message_rect.y = ypos+5;
        Message_rect.w = 100; // controls the width of the rect
        Message_rect.h = 35; // controls the height of the rect
        SDL_RenderCopy(context->renderer, Message, nullptr, &Message_rect);
        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);
    }

    void setOnClickHandler(void(*function_pointer)())
    {
        onClick_functionPTR = function_pointer;
    }

    void onClick(Renderer* RENDERER) override//When button is selected, run the click handler
    {
        if(onClick_functionPTR != nullptr) {
            (*onClick_functionPTR)();//Do what needs to be done. TODO: have onClick return the value from onClick_functionPTR
        }
    }

};




////End Widgets/////
class Renderer {
private:
    //Define the SDL environment variables
    SDL_Window *window = nullptr;

    TTF_Font *Sans = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 45);
    TTF_Font *SansOilGaugeTicks = TTF_OpenFont("/home/dylan/Desktop/sans/OpenSans-Regular.ttf", 45);
public:
    SDL_Renderer *renderer = nullptr;
    menuPage *PAGES[5]{};
    menuPage *currentPage = nullptr;
    menuPage *previousPage = nullptr;
//    editPage EDITOR = editPage();
    int pageCount = 0;

    Renderer(SDL_Window *WINDOW, SDL_Renderer *RENDERER) {
        window = WINDOW;
        renderer = RENDERER;
    }
    void addPage(menuPage* inputPage);
    void incrementSelectedWidget()
    {
        currentPage->incrementSelectedWidget();
    }
    void decrementSelectedWidget()
    {
        currentPage->decrementSelectedWidget();
    }
    void back()
    {
        currentPage = previousPage;
    }
    void render();
};

void Renderer::addPage(menuPage *inputPage) {
    //set the inputPage widgets parentRenderer property
    if(pageCount == 0)
    {
        currentPage = inputPage;
    }
    PAGES[pageCount] = inputPage;
  pageCount++;
}

void loadPage(Renderer* renderer,menuPage* page) {//Helper function, loads and changes renderer current page.
    renderer->previousPage = renderer->currentPage;
    renderer->currentPage = page;
}

struct Context* getContext(Renderer* RENDERER)
{
    auto* tempContext = (struct Context*)malloc(sizeof(struct Context));
    tempContext->RENDERER = RENDERER;
    tempContext->currentPage = RENDERER->currentPage;
    tempContext->renderer = RENDERER->renderer;
    return tempContext;
}

void Renderer::render() {
    //Draw the page Title
    SDL_Color textColor = {255, 255, 255};
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, currentPage->title, textColor); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_Rect Message_rect; //create a rect
    Message_rect.x = 0+(Window_Width/2)-(200/2);
    Message_rect.y = 25;
    Message_rect.w = 200; // controls the width of the rect
    Message_rect.h = 45; // controls the height of the rect
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_DestroyTexture(Message);
    SDL_FreeSurface(surfaceMessage);
    //DrawRectangle_FIX(renderer,&gridRectangle);
    //End page title drawing
    for(int index = 0; index < currentPage->widgetVector.size(); index++)
    {
        if(currentPage->widgetVector[index].widgetType == DASHBOARD_WIDGET)
        {
            auto* widPTR = (DashboardWidget*)currentPage->widgetVector[index].widgetPTR;//Load the widget from the currentPage
            if(currentPage->selectedItem == index)
            {
                widPTR->isSelected = true;
            }
            else {
                widPTR->isSelected = false;
            }
            widPTR->onDraw(this);
        }
    }
}
///////////////////End Renderer Functions////////////////////////