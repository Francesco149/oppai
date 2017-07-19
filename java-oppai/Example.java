import dp.oppai.*;
public class Example {
	public static boolean checkError(OppaiCtx ctx) {
		// Oppai doesn't throw an exception when errors occur, but gives you the errors as strings, similar to what windows or linux for example do
		// Feel free to make your own Exception and handle this however you like.
		String err = ctx.getLastErr(); 
		
		if(!err.equals("")) {
			System.out.println(err);
			return true;
		}
		return false;
	}
	
	public static void printDiff(double stars, double aim, double speed) {
		System.out.printf("\n%.2f stars\n%.2f aim stars\n%.2f speed stars", stars, aim, speed);
	}
	
	public static void printPP(double acc, double pp, double aimPP, double speedPP, double accPP) {
		System.out.printf("\n%.2f aim\n%.2f speed\n%.2f acc\n%.2f pp\nfor %.2f%%", aimPP, speedPP, accPP, pp, acc );
	}
	
	public static void main(String[] args) {
		if(args.length != 1) {
			System.out.println("Usage: java -Djava.library.path=\"path/to/shared/lib;${env_var:PATH}\" -cp '.:path/to/jar' path/to/main file.osu");
			return;
		}
		
		// If you need to multithread, create one ctx and buffer for each thread
		OppaiCtx ctx = new OppaiCtx();
		
		// Parse beatmap ------------------------------------------------------------
		Beatmap b = new Beatmap(ctx);
		
		int BUFFER_SIZE = 8000000; // Should be big enough to hold the .osu file
		Buffer buf = new Buffer(BUFFER_SIZE);
		
		b.parse(args[0], buf, false, System.getProperty("user.dir")); // Don't disable caching and use the current working directory for caching
		
		if(checkError(ctx))
			return;
		
		// Dispose of our buffer, since we no longer need it. If we want to eventually parse a new beatmap we should create a new buffer.
		// Sadly it's hard to get rid of our native objects with Java's garbage collector and no ideal alternative is given in java (like C++'s destructors)
		// so this method is needed and it's very recommended to call it if you don't want memory leaks.
		// the buf object itself still exists in this scope and only the native one no longer exists, so make sure you don't pass this object to any function
		// anymore because the underlying native object will not exist and this will crash your program.
		buf.dispose();
		
		System.out.println("Cache foler: " + System.getProperty("user.dir"));
		
		float cs = b.getCS(), od = b.getOD(), ar = b.getAR(), hp = b.getHP();
		
		System.out.println(b); // Prints a nicely formatted message of the overall metadata of the beatmap
		
		// diff calc ----------------------------------------------------------------
		
		OppaiCtx dctx = new OppaiCtx(ctx); // Passing a ctx object will create a diff ctx object
		
		b.diffCalc(dctx, true, true, true, true);
		if(checkError(ctx))
			return;
		
		printDiff(b.getStars(), b.getAim(), b.getSpeed());
		
		// pp calc ------------------------------------------------------------------
		b.calcPP(ctx, b.getAim(), b.getSpeed());
		
		if(checkError(ctx))
			return;
		
		printPP(b.getAccPercent(), b.getPp(), b.getAimPP(), b.getSpeedPP(), b.getAccPP());
		
		// pp calc (with acc %) -----------------------------------------------------
		b.calcPPAcc(ctx, b.getAim(), b.getSpeed(), 90.0);
		
		if(checkError(ctx))
			return;
		
		printPP(b.getAccPercent(), b.getPp(), b.getAimPP(), b.getSpeedPP(), b.getAccPP());
		
		// override OD example ------------------------------------------------------
		System.out.println("\n----\nIf the map was od10:");
		b.setOD(10);
		
		b.calcPP(ctx, b.getAim(), b.getSpeed());
		
		if(checkError(ctx))
			return;
		
		printPP(b.getAccPercent(), b.getPp(), b.getAimPP(), b.getSpeedPP(), b.getAccPP());
		
		b.setOD(od);
		
		// override AR example ------------------------------------------------------
		System.out.println("\n----\nIf the map was ar11:");
		b.setAR(11);
		
		b.calcPP(ctx, b.getAim(), b.getSpeed());
		
		if(checkError(ctx))
			return;
		
		printPP(b.getAccPercent(), b.getPp(), b.getAimPP(), b.getSpeedPP(), b.getAccPP());
		
		b.setAR(ar);
		// override CS example ------------------------------------------------------
		System.out.println("\n----\nIf the map was cs6.5:");
		b.setCS(6.5f);
		
		// remember that CS is map-changing so difficulty must be recomputed
		b.diffCalc(dctx, true, true, true, true);
		
		if(checkError(ctx))
			return;
		
		printDiff(b.getStars(), b.getAim(), b.getSpeed());
		
		b.calcPP(ctx, b.getAim(), b.getSpeed());
		
		if(checkError(ctx))
			return;
		
		printPP(b.getAccPercent(), b.getPp(), b.getAimPP(), b.getSpeedPP(), b.getAccPP());
		
		b.setCS(cs);
		
		// mods example -------------------------------------------------------------
		System.out.println("\n----\nWith HDHR:");
		
		// mods are a bitmask, same as what the osu! api uses
	    int mods = Mods.HD | Mods.HR;
		b.applyMods(mods);
		
		// Some mods (like HR) are map-changing, so we need to recompute the diff
		b.diffCalc(dctx, true, true, true, true);
		
		if(checkError(ctx))
			return;
		
		printDiff(b.getStars(), b.getAim(), b.getSpeed());
		
		b.calcPP(ctx, b.getAim(), b.getSpeed(), mods);
		
		if(checkError(ctx))
			return;
		
		printPP(b.getAccPercent(), b.getPp(), b.getAimPP(), b.getSpeedPP(), b.getAccPP());
		
		// Dispose of our contexts and beatmap, since we no longer needed. 
		b.dispose();
		ctx.dispose();
		dctx.dispose();
	}
}
