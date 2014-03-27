// The object in our scene that our camera is currently tracking.
private var target : Transform;

// This is for setting interpolation on our target, but making sure we don't permanently
// alter the target's interpolation setting.  This is used in the SetTarget () function.
private var savedInterpolationSetting = RigidbodyInterpolation.None;

function SetTarget (newTarget : Transform, snap : boolean) 
{
	// If there was a target, reset its interpolation value if it had a rigidbody.
	if  (target) 
	{
		// Reset the old target's interpolation back to the saved value.
		targetRigidbody = target.GetComponent (Rigidbody);
		if  (targetRigidbody)
			targetRigidbody.interpolation = savedInterpolationSetting;
	}
	
	// Set our current target to be the value passed to SetTarget ()
	target = newTarget;
	
	// Now, save the new target's interpolation setting and set it to interpolate for now.
	// This will make our camera move more smoothly.  Only do this if we didn't set the
	// target to null (nothing).
	if (target) 
	{
		targetRigidbody = target.GetComponent (Rigidbody);
		if (targetRigidbody) 
		{
			savedInterpolationSetting = targetRigidbody.interpolation;
			targetRigidbody.interpolation = RigidbodyInterpolation.Interpolate;
		}
	}
}

// Provide another version of SetTarget that doesn't require the snap variable to set.
// This is for convenience and cleanliness.  By default, we will not snap to the target.
function SetTarget (newTarget : Transform) 
{
	SetTarget (newTarget, false);
}

// This is a simple accessor function, sometimes called a "getter".  It is a publically callable
// function that returns a private variable.  Notice how target defined at the top of the script
// is marked "private"?  We can not access it from other scripts directly.  Therefore, we just
// have a function that returns it.  Sneaky!
function GetTarget () 
{
	return target;
}


