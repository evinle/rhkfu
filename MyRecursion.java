package recursivity;

import java.util.*;

public class MyRecursion {
	
	public static void main( String[] args ) {
		
		Scanner scanIn = new Scanner( System.in );
		String input = new String();
		int intInput = 0;
		int fibNum = 0;
		int choice = 0;
		
		// loop the menu until the end condition is met, which is when a number 
		// that is not 1 is entered as the choice
		do{
			System.out.println( "Please enter the fibonacci element that you want"
				+ " to see:");
			
			// do this so we can verify that it is an integer indeed in the try 
			// catch without crashing the program
			input = scanIn.nextLine();
			
			try {
				// parse the input into int and see if it is an integer indeed
				intInput = Integer.parseInt( input ); 
				// if tis indeed not an of type int, it shall be caught by the 
				// catch statement
				
				// once we know it is an int, we need to make sure that it's a 
				// valid fibonacci number, i.e > 0
				if( intInput <= 0 ) {
					throw new IllegalArgumentException( "Please enter a positive"
						+ " number");
				}
				
				fibNum = fibRec( intInput );
							
				System.out.println( fibNum );
				
				System.out.println( "Would you like to continue?\nIf yes"+ 
					": press 1\nElse: press any other number" );
				
				// condition for loop to continue is 1
				choice = Integer.parseInt( scanIn.nextLine() );
				
				// clear input of "input" so as to not to have its value 
				// remaining when the loop loopty loops
				input = new String();
			}
			catch( NumberFormatException e ) {
				System.out.println( "You did not enter an integer, the program will"
					+ " now terminate" );
				choice = 0;
			}
			catch( IllegalArgumentException e2 ){
				System.out.println( "Error detected: " + e2 );
				choice = 0;
			}			
						
		} while ( choice == 1 );
	}
	
	public static int fibRec( int n ){
		
		int returnFib = 0;
		
		// hardcode the first 2
		if( n == 1 ){
			returnFib = 0;	
		}
		else if ( n == 2 ){
			returnFib = 1;
		}
		else {
			returnFib = fibRec( n - 1 ) + fibRec( n - 2 );
			// self explanatory i hope, but if not, value of a fibonacci number
			// is equal to the sum of the its two predecessors 
		}
		
		return returnFib;
	}
}
