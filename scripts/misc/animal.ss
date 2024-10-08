// -----------------------------------------------------------------------------
// File: animal.ss
// Description: little animal script
// Author: Alexandre Martins <http://opensurge2d.org>
// License: MIT
// -----------------------------------------------------------------------------
using SurgeEngine.Actor;
using SurgeEngine.Level;
using SurgeEngine.Transform;
using SurgeEngine.Behaviors.Platformer;
using SurgeEngine.Behaviors.DirectionalMovement;

// a generic little animal (usually spawned when defeating baddies)
object "Animal" is "entity", "private", "disposable"
{
    public id = 0; // animal ID: 0, 1, 2, 3...
    public secondsBeforeMoving = 0.0;
    public zindex = 0.5;

    actor = Actor("Animal");
    transform = Transform();
    movement = null;
    activationState = null;
    maxAnimals = 1;

    state "main"
    {
        // play the "appearing" animation
        actor.anim = idToAnim(id);
        actor.zindex = zindex;

        // wait a bit
        state = "wait";
    }

    state "wait"
    {
        if(timeout(secondsBeforeMoving)) {
            movement.enabled = true;
            state = activationState;
        }
    }

    state "start jumping"
    {
        platformer = movement;
        updatePlaformerSettings(platformer);

        platformer.forceJump(platformer.jumpSpeed * 1.1);
        state = "initial jump";
    }

    state "initial jump"
    {
        platformer = movement;
        updatePlaformerSettings(platformer);

        // when touching the ground, run!
        if(!platformer.midair) {
            if(Math.random() < 0.5)
                platformer.walkLeft();
            else
                platformer.walkRight();

            actor.anim++; // play "running" animation
            state = "running";
        }
    }

    state "running"
    {
        platformer = movement;
        updatePlaformerSettings(platformer);

        if(platformer.rightWall)
            platformer.walkLeft();
        else if(platformer.leftWall)
            platformer.walkRight();
        else if(!platformer.midair)
            platformer.jump();
    }

    state "start flying"
    {
        birdMovement = movement;

        actor.anim++; // play "flying" animation
        actor.hflip = (birdMovement.direction.x < 0);
        state = "flying";
    }

    state "flying"
    {
    }

    fun constructor()
    {
        // find the Animal theme manager
        animalManager = Level.child("Animals") || Level.spawn("Animals");

        // read maxAnimals
        maxAnimals = animalManager.maxAnimals;

        // pick a random animal
        for(i = 0; i < 10; i++) {
            id = animalManager.pickAnimal();

            // check if the required animation has been defined in the .spr file
            actor.anim = idToAnim(id);
            if(actor.animation.exists)
                break;
        }

        // select the proper movement
        if(animalManager.animalType(id) == "bird") {
            // setup the bird movement
            birdMovement = DirectionalMovement();
            birdMovement.speed = 64;
            birdMovement.angle = 30 * Math.random() + 15;
            birdMovement.angle = (Math.random() > 0.5) ? 180 - birdMovement.angle : birdMovement.angle;

            movement = birdMovement;
            activationState = "start flying";
        }
        else {
            // setup the platformer
            platformer = Platformer();
            updatePlaformerSettings(platformer);

            movement = platformer;
            activationState = "start jumping";
        }

        // won't move initially
        movement.enabled = false;
    }

    fun updatePlaformerSettings(platformer)
    {
        platformer.speed = 64;
        platformer.jumpSpeed = 200;
        platformer.gravityMultiplier = 1.0;
        actor.animation.speedFactor = 1.0;

        if(isUnderwater()) {
            platformer.speed /= 2.0;
            platformer.jumpSpeed /= 1.85;
            platformer.gravityMultiplier = 0.2857;
            actor.animation.speedFactor = 0.7;
        }
    }

    fun isUnderwater()
    {
        return (transform.position.y >= Level.waterlevel);
    }

    fun idToAnim(id)
    {
        return 2 * Math.clamp(id, 0, maxAnimals - 1);
    }
}

// Object Animals (plural) is responsible for managing the animal theme
// for the current level (i.e., which animals can be spawned in the
// current level). The theme can be defined in a setup script.
object "Animals"
{
    // The theme is an array of indices indicating
    // which animals can be spawned (see the sprite)
    theme = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]; // default theme

    // the type of each animal (jumper, bird, etc.)
    indexedType = [
        "jumper",   // 0
        "jumper",   // 1
        "jumper",   // 2
        "jumper",   // 3
        "jumper",   // 4
        "jumper",   // 5
        "jumper",   // 6
        "jumper",   // 7
        "jumper",   // 8
        "jumper",   // 9
        "jumper",   // 10
        "jumper",   // 11
        "jumper",   // 12
        "jumper",   // 13
        "jumper",   // 14
        "bird",     // 15
        "jumper",   // 16
        "bird",     // 17
    ];

    // give me a random animal ID
    fun pickAnimal()
    {
        index = Math.floor(Math.random() * theme.length);
        return theme[index];
    }

    // given an animal ID, return its type
    fun animalType(id)
    {
        if(id >= 0 && id < indexedType.length)
            return indexedType[id];
        else
            return indexedType[0];
    }

    // how many animals are available?
    fun get_maxAnimals()
    {
        return indexedType.length;
    }

    // get theme
    fun get_theme()
    {
        return theme;
    }

    // set theme
    fun set_theme(newTheme)
    {
        if(typeof newTheme == "object" && newTheme.__name == "Array" && newTheme.length > 0)
            theme = newTheme;
        else
            Console.print("Animals.theme must be an array of indices");
    }
}