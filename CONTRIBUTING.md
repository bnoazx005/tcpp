## Foreword

First of all, thank you for you patience and attention for this project. We glad to see any new developer here. Hope that you've found out the project is useful for you. So if you want to contribute and support it, please, read this guide first before your hands get dirty.

As I wrote above we glad to work with any developer with different skills level. If you have no enough experience in C# development you can work on documentation (e.g. write tutorials), write bug reports, etc. **good first issue** label in Issues tab will be the best entry point for all newcomers. 

## Typical workflow

***

* Design a feature.

* Decompose it into a list of tasks.

* Write code.

* Cover written code with bunch of unit tests

* Commit the result to the repository of the project.

When we decide that the feature is ready for integration into the main build we firstly merge it into test-build. After successfull build of the branch later it can be merged into master. Usually master branch contains only final releases of the project. 

## Styleguides

***

### Git commit messages styleguide

In the project we try to stick to notation which was described at [this](https://chris.beams.io/posts/git-commit/) great article. 

We also use some kind of notation for names of branches. All branches that introduce a new feature should start from _**feature/branch-name**_. The following rules are used for other types of branches:

* _**fix/branch-name**_ - fix some issue.

* _**refactoring[/branch-name]**_ - the code in this branch refactors, optimize some part of the project.

* _**test-build**_ - before the changes all the developers have introduced will appear in **master** branch they should be tested. We use this branch for this purpose.

### C\+\+ Styleguide

* Be sure you use single tab for indentation.

* All interfaces names should start from **I** prefix (e.g. **IManager**). Classes don't need any prefix (e.g. **Manager**). Structures identifiers starts from **T** prefix (e.g. **TVector2**). Names of classes, interfaces and structures should stick to camel case (e.g. **ComplexNameOfSomeClass**).

* Enumerations' names start from **E_** prefix and stick to snake case. For instance, **E_MESSAGE_TYPE**.

* Names of global variables and constants should start from Upper case, for instance **SingletonInstance**.

* All local variables should start from lower case (**someLocalVariable**). Be sure choose some proper and clear names for them. But remember there is no special notation for names except described above.

* Members of classes should start from *m* prefix (e.g. "mIsEnabled") including static variables. All public methods names should start from Upper case and stick to camel case. Protected and private methods start from **_** prefix (e.g. **_somePrivateMethod**). The same rules are applicable for properties both public, private (protected). Names of public events start from **On** prefix. **Is** prefix are appreciated for methods that are logical predicates that tell to a user whether some variable is true or false.

* Stick to Allman style of indentation. For instance
	```csharp
	//...
	while (true)
	{
		DoSomething1();
		//...
		DoSomething2();
	}
	//...
	```
* Single-statement block should be wrapped in braces too. Also add extra space after operators like **if**, **for**, **switch** and etc. The extra space isn't used in case of method's invocation.
	```csharp
	// Wrong (DoSomething() call should be wrapped with braces)
	while (true)
		DoSomething();

	// Wrong (needs extra space after while)
	while(true)
	{
		DoSomething();
	}

	// Wrong (function call doesn't need extra space)
	while (true)
	{
		DoSomething ();
	}

	// Right!
	while (true)
	{
		DoSomething();
	}
	```

These are the main rules of the notation that's used within the project. Hope that nothing important wasn't missed) If you have some questions that aren't mentioned here either write [e-mail](mailto:ildar2571@yandex.ru) or send message into our [gitter](https://gitter.im/bnoazx005/TinyECS).