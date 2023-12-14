import cv2
import numpy as np
import time
import os
import Cards
import VideoStream
import serial
from time import sleep
from threading import Thread
 

## Camera settings
IM_WIDTH = 1280
IM_HEIGHT = 720 
FRAME_RATE = 10

# Dimensions of rank train images
RANK_WIDTH = 70
RANK_HEIGHT = 125

# Dimensions of suit train images
SUIT_WIDTH = 70
SUIT_HEIGHT = 100

# Serial
serial_port = 'COM29'
baud_rate = 9600

# Cards
cardsAllowed = [0]

### Structures to hold query card and train card information ###

videostream = VideoStream.VideoStream((IM_WIDTH,IM_HEIGHT),FRAME_RATE,2,0).start()
imageGlobal = videostream.read()

class Query_card:
    """Structure to store information about query cards in the camera image."""

    def __init__(self):
        self.contour = [] # Contour of card
        self.width, self.height = 0, 0 # Width and height of card
        self.corner_pts = [] # Corner points of card
        self.center = [] # Center point of card
        self.warp = [] # 200x300, flattened, grayed, blurred image
        self.rank_img = [] # Thresholded, sized image of card's rank
        self.suit_img = [] # Thresholded, sized image of card's suit
        self.best_rank_match = "Unknown" # Best matched rank
        self.best_suit_match = "Unknown" # Best matched suit
        self.rank_diff = 0 # Difference between rank image and best matched train rank image
        self.suit_diff = 0 # Difference between suit image and best matched train suit image

class Train_ranks:
    """Structure to store information about train rank images."""

    def __init__(self):
        self.img = [] # Thresholded, sized rank image loaded from hard drive
        self.name = "Placeholder"

class Train_suits:
    """Structure to store information about train suit images."""

    def __init__(self):
        self.img = [] # Thresholded, sized suit image loaded from hard drive
        self.name = "Placeholder"


## Initialize calculated frame rate because it's calculated AFTER the first time it's displayed
frame_rate_calc = 1
freq = cv2.getTickFrequency()

# Load the train rank and suit images
path = os.path.dirname(os.path.abspath(__file__))
train_ranks = Cards.load_ranks( path + '/Card_Imgs/')
train_suits = Cards.load_suits( path + '/Card_Imgs/')

# Serial Methods


def listen_ard(ser):
    #if ser.in_waiting > 0:
    print('arduino received:')
        
    #global imageGlobal
    while True:
        #imageGlobal = videostream.read()
        incoming_message = ser.readline().decode().strip()
        print(incoming_message)
        if incoming_message == "CARD_ALLOWED_REQUEST":
            print('card request')
            allowed = cardMode(imageGlobal)
            send_data(ser, allowed)
        elif incoming_message[0:14] == "EXCLUDED_CARDS":
            print('cards allowed')
            cardsAllowed = incoming_message[14:].split()
            print(cardsAllowed)
        else:
            print('unknow msg')
            
            

def send_data(ser, cardAllowed):
    allowed = 'FALSE'
    if cardAllowed:
        allowed = 'TRUE'
    try:
        ser.write(f"CARD_ALLOWED_RESPONSE {allowed}\n".encode('utf-8'))
        print(f"CARD_ALLOWED_RESPONSE {allowed}\n".encode('utf-8'))
    except Exception as e:
        print(f"Error sending data to Arduino: {e}")


def detect(image):
    
    # Initialize new Query_card object
    qCard = Query_card()
    # Pre-process camera image (gray, blur, and threshold it)
    pre_proc = Cards.preprocess_image(image)
    
    crop = pre_proc[0:500, 0:200]
    crop = invert = cv2.bitwise_not(crop)
    cv2.imshow("cropp",crop)
    
    # Split in to top and bottom half (top shows rank, bottom shows suit)
    Qrank = crop[80:280, 20:180]
    Qsuit = crop[300:460, 20:180]
    cv2.imshow("rank",Qrank)
    cv2.imshow("suit",Qsuit)
    
    # Find rank contour and bounding rectangle, isolate and find largest contour
    Qrank_cnts, hier = cv2.findContours(Qrank, cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    Qrank_cnts = sorted(Qrank_cnts, key=cv2.contourArea,reverse=True)
    
    # Find bounding rectangle for largest contour, use it to resize query rank
    # image to match dimensions of the train rank image
    if len(Qrank_cnts) != 0:
        x1,y1,w1,h1 = cv2.boundingRect(Qrank_cnts[0])
        Qrank_roi = Qrank[y1:y1+h1, x1:x1+w1]
        Qrank_sized = cv2.resize(Qrank_roi, (RANK_WIDTH,RANK_HEIGHT), 0, 0)
        qCard.rank_img = Qrank_sized
        cv2.imshow("rank2",Qrank_roi)

    # Find suit contour and bounding rectangle, isolate and find largest contour
    Qsuit_cnts, hier = cv2.findContours(Qsuit, cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    Qsuit_cnts = sorted(Qsuit_cnts, key=cv2.contourArea,reverse=True)
    
    # Find bounding rectangle for largest contour, use it to resize query suit
    # image to match dimensions of the train suit image
    if len(Qsuit_cnts) != 0:
        x2,y2,w2,h2 = cv2.boundingRect(Qsuit_cnts[0])
        Qsuit_roi = Qsuit[y2:y2+h2, x2:x2+w2]
        Qsuit_sized = cv2.resize(Qsuit_roi, (SUIT_WIDTH, SUIT_HEIGHT), 0, 0)
        qCard.suit_img = Qsuit_sized
        cv2.imshow("suit2",Qsuit_roi)
        
    
    # Finally, display the image with the identified cards!
    cv2.imshow("Card Detector",image)
    #cv2.imshow("Preproc",pre_proc)
    
    
    # Initialize a new "cards" list to assign the card objects.
    # k indexes the newly made array of cards.
    cards = [qCard]
    k = 0
        
    # Find the best rank and suit match for the card.
    cards[k].best_rank_match,cards[k].best_suit_match,cards[k].rank_diff,cards[k].suit_diff = Cards.match_card(cards[k],train_ranks,train_suits)
    
    best_rank_match_diff = 10000
    best_suit_match_diff = 10000
    best_rank_match_name = "Unknown"
    best_suit_match_name = "Unknown"
    
    if (len(qCard.rank_img) != 0) and (len(qCard.suit_img) != 0):
        
        # Difference the query card rank image from each of the train rank images,
        # and store the result with the least difference
        for Trank in train_ranks:

                diff_img = cv2.absdiff(qCard.rank_img, Trank.img)
                rank_diff = int(np.sum(diff_img)/255)
                
                if rank_diff < best_rank_match_diff:
                    best_rank_diff_img = diff_img
                    best_rank_match_diff = rank_diff
                    best_rank_name = Trank.name

        # Same process with suit images
        for Tsuit in train_suits:
                
                diff_img = cv2.absdiff(qCard.suit_img, Tsuit.img)
                suit_diff = int(np.sum(diff_img)/255)
                
                if suit_diff < best_suit_match_diff:
                    best_suit_diff_img = diff_img
                    best_suit_match_diff = suit_diff
                    best_suit_name = Tsuit.name

    # Combine best rank match and best suit match to get query card's identity.
    # If the best matches have too high of a difference value, card identity
    # is still Unknown
    if (best_rank_match_diff < 3500):
        best_rank_match_name = best_rank_name

    if (best_suit_match_diff < 3500):
        best_suit_match_name = best_suit_name
        
        
    #print(cards[k].best_rank_match,cards[k].best_suit_match,cards[k].rank_diff,cards[k].suit_diff)
    
    #print(best_rank_match_name, best_suit_match_name)
    
    return cards[k].best_rank_match, cards[k].best_suit_match
    #print(cards[k].best_rank_match,cards[k].best_suit_match)
    # Draw center point and match result on the image.


def cardMode(image):
    global imageGlobal
    ranks = []
    suits = []
    t_end = time.time() + 1
    while time.time() < t_end:
        imageGlobal = videostream.read()
        rank, suit = detect(imageGlobal)
        ranks.append(rank)
        suits.append(suit)
        
    print(max(set(ranks), key=ranks.count), max(set(suits), key=suits.count))
    return cardAllowed(max(set(ranks), key=ranks.count), max(set(suits), key=suits.count))
    
def cardAllowed(rank, suit):
    nRank = 0
    nSuit = 0
    
    if rank == 'Ace':
        nRank = 1
    elif rank == 'Two':
        nRank = 2
    elif rank == 'Three':
        nRank = 3
    elif rank == 'Four':
        nRank = 4
    elif rank == 'Five':
        nRank = 5
    elif rank == 'Six':
        nRank = 6
    elif rank == 'Seven':
        nRank = 7
    elif rank == 'Eight':
        nRank = 8
    elif rank == 'Nine':
        nRank = 9
    elif rank == 'Ten':
        nRank = 10
    elif rank == 'Jack':
        nRank = 11
    elif rank == 'Queen':
        nRank = 12
    elif rank == 'King':
        nRank = 13
        
    if suit == 'Clubs':
        nSuit = 0
    elif suit == 'Hearts':
        nSuit = 1
    elif suit == 'Spades':
        nSuit = 2
    elif suit == 'Diamonds':
        nSuit = 3
    
    card = nRank + nSuit*13
    print(card)
    
    if card in cardsAllowed:
        return False
    
    return True
    
def main ():
    print("INIT")
    global imageGlobal
    cam_quit = 0 # Loop control variable
    
    
    #with serial.Serial(serial_port, baud_rate) as ser:  
        #listen_ard(ser)
    

    # Begin capturing frames
    while cam_quit == 0:
        imageGlobal = videostream.read()
        cv2.imshow("image", imageGlobal)
            
        #cardMode(pre_proc)
       
            
        # Poll the keyboard. If 'q' is pressed, exit the main loop.
        key = cv2.waitKey(1) & 0xFF
        if key == ord("q"):
            cam_quit = 1
        elif key == ord("d"):
            cardMode(imageGlobal)
                
                
if __name__ == "__main__":
    main()

# Close all windows and close the PiCamera video stream.
cv2.destroyAllWindows()
videostream.stop()