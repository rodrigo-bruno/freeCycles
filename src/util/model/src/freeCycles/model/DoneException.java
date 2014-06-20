package freeCycles.model;

/**
 * @author underscore
 * Exception thrown by the server when no more work has to be done.
 * I know, exceptions are for non expected scenarios... But I like to do this
 * funny non-local control transfer =).
 */
public class DoneException extends RuntimeException {

	private static final long serialVersionUID = 1L;
	
}
